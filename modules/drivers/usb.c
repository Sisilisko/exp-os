


#include "types.h"
#include "pci.h"
#include "mmio.h"
#include "memory.h"

/* Pause helper / memory barrier */
static inline void cpu_pause(void) { __asm__ volatile("pause"); }
static inline void memory_barrier(void) { __asm__ volatile("" ::: "memory"); }

/* Simple debug output (implement as you like) */
extern void klog(const char *fmt, ...);

/* -----------------------------
   xHCI register offsets and constants (subset)
   Ref: xHCI spec (abbreviated)
   -----------------------------*/
#define XHCI_CAPLENGTH        0x00 /* Capability Length (8-bit) */
#define XHCI_HCSPARAMS1       0x04
#define XHCI_HCSPARAMS2       0x08
#define XHCI_HCSPARAMS3       0x0C
#define XHCI_USBCMD           0x00  /* Operational registers (offset = CAPLENGTH) */
#define XHCI_USBSTS           0x04
#define XHCI_PAGESIZE         0x08
#define XHCI_DNCTRL           0x14
#define XHCI_CRCR_LOW         0x18
#define XHCI_CRCR_HIGH        0x1C
#define XHCI_DCBAAP_LOW       0x30
#define XHCI_DCBAAP_HIGH      0x34
#define XHCI_CONFIG           0x38

/* runtime regs */
#define XHCI_RUNTIME_OFFSET   0x00002000 /* typical; computed from capability regs */

/* Doorbell array is located after operational and runtime blocks at runtime_offset + doorbell
   but we will compute runtime_offset from capability register (HCCPARAMS?). */

/* USB Command register flags */
#define USBCMD_RUN_STOP       (1 << 0)
#define USBCMD_HCRST          (1 << 1)
#define USBCMD_INTE           (1 << 2)
#define USBCMD_HSEE           (1 << 3)
#define USBCMD_LTC            (1 << 4)

/* USB Status flags */
#define USBSTS_HCH            (1 << 0)
#define USBSTS_HCE            (1 << 11)

/* Event TRB types */
#define TRB_TYPE_NORMAL       1
#define TRB_TYPE_SETUP_STAGE  2
#define TRB_TYPE_DATA_STAGE   3
#define TRB_TYPE_STATUS_STAGE 4
#define TRB_TYPE_COMMAND_COMPLETION 33
#define TRB_TYPE_TRANSFER_EVENT     32

/* TRB flags */
#define TRB_RING_CYCLE        (1ULL << 0)
#define TRB_TT                    (0<<10)

/* Simplified TRB struct */
struct xhci_trb {
    uint64_t parameter; /* pointer or immediate */
    uint32_t status;
    uint32_t control;
} __attribute__((packed, aligned(16)));

#define TRB_CONTROL_TYPE(c)   (((c) >> 10) & 0x3F)
#define TRB_CONTROL_SID       (1u<<0)   /* for command TRB etc */
#define TRB_CONTROL_ENT       (1u<<1)   /* evaluate next TRB? */
#define TRB_CONTROL_IOC       (1u<<5)   /* interrupt on completion */

/* Event TRB structure (32 bytes) */
struct xhci_event_trb {
    uint64_t param;
    uint32_t status;
    uint32_t control;
} __attribute__((packed, aligned(16)));

/* A very small ring implementation for Command Ring and Event Ring */
#define COMMAND_RING_TRBS  256
#define EVENT_RING_TRBS    256

/* xHCI driver state */
struct xhci {
    pci_t *pci;
    void *mmio;             /* mapped base of xHCI registers */
    uint32_t caplength;
    void *op_regs;          /* operational base (mmio + caplength) */
    void *runtime_regs;     /* runtime regs base */
    void *doorbell_array;   /* doorbells base (runtime + doorbell offset) */

    /* Command ring */
    struct xhci_trb *cmd_ring;
    uint32_t cmd_ring_size;
    uint32_t cmd_ring_cycle;

    /* Event ring */
    void *erst;             /* Event Ring Segment Table (virt addr) */
    struct xhci_event_trb *event_ring;
    uint32_t event_ring_size;
    uint32_t event_ring_consumer_index;
    uint8_t event_ring_cycle;

    /* DCBAA (Device Context Base Address Array) */
    uint64_t *dcbaa;
    /* other state like slot contexts, device contexts etc would go here */
};

static struct xhci g_xhci;

/* -----------------------------
   PCI scanning for xHCI controller (class 0x0C subsystem 3)
   We'll find vendor/device where class = 0x0C and subclass = 0x03
   (USB controller; Program Interface 0x30 for xHCI typically)
   -----------------------------*/

static bool pci_find_xhci(struct pci_addr *out)
{
    /* VERY simple: iterate a few buses/devs; replace with your PCI enumeration */
    for (uint8_t bus = 0; bus < 8; ++bus) {
        for (uint8_t dev = 0; dev < 32; ++dev) {
            for (uint8_t fn = 0; fn < 1; ++fn) {
                struct pci_addr *p = {bus, dev, fn};
                uint32_t hdr0 = pci_read_config32(p->bus, p->slot, p->function, 0x08); /* class/prog-if/etc at offset 0x08 */
                uint8_t baseclass = (hdr0 >> 24) & 0xFF;
                uint8_t subclass = (hdr0 >> 16) & 0xFF;
                uint8_t prog_if = (hdr0 >> 8) & 0xFF;
                if (baseclass == 0x0C && subclass == 0x03) {
                    /* Found USB controller; check if xHCI (prog IF 0x30 often) */
                    klog("pci: found USB controller @ %u:%u.%u prog-if 0x%02x\n", bus, dev, fn, prog_if);
                    out->bus = p;
                    return true;
                }
            }
        }
    }
    return false;
}

/* Read BAR0 from PCI and map MMIO */
static bool xhci_map_mmio(struct xhci *xh)
{
    uint32_t bar0 = pci_read_config32(xh->pci->bus, xh->pci->slot, xh->pci->function,  0x10);
    /* BAR may be 64-bit; we read only low dword for simplicity. Clear low bits */
    uint64_t mmio_phys = (uint64_t)(bar0 & ~0x0F);
    if (!mmio_phys) return false;
    /* Map at least 64k of MMIO (practical controllers require more; query BAR size for production) */
    xh->mmio = mmio_map_phys(mmio_phys, 0x10000);
    if (!xh->mmio) return false;

    uint8_t caplength = *(volatile uint8_t *)((char*)xh->mmio + XHCI_CAPLENGTH);
    xh->caplength = caplength;
    xh->op_regs = (void *)((char*)xh->mmio + caplength);
    /* runtime regs are at offset from cap registers: read HCSPARAMS1/2? We'll compute common offset 0x2000 */
    xh->runtime_regs = (void *)((char*)xh->mmio + 0x2000); /* conservative default; many controllers use this */
    /* doorbell array follows runtime -> runtime + 0x1000 typically; compute by reading RTR */
    xh->doorbell_array = (void *)((char*)xh->runtime_regs + 0x1000); /* heuristic; adjust per spec for production */
    klog("xhci: caplen=%u op_reg=%p runtime=%p doorbell=%p\n", caplength, xh->op_regs, xh->runtime_regs, xh->doorbell_array);
    return true;
}

/* -----------------------------
   Command Ring creation & control
   -----------------------------*/
static bool xhci_create_command_ring(struct xhci *xh)
{
    xh->cmd_ring_size = COMMAND_RING_TRBS;
    xh->cmd_ring = (struct xhci_trb *)rmap(1); /* 4096 bytes -> fits 256*16 = 4096 */
    if (!xh->cmd_ring) return false;
    /* Zero memory assumed by rmap */
    xh->cmd_ring_cycle = 1;

    /* Setup CRCR (Command Ring Control Register) to point to ring phys addr and set cycle state.
       For simplicity, assume identity mapping of rmap (physical == virtual),
       otherwise you'd need to provide physical address. */

    uint64_t cr_phys = (uint64_t)(uintptr_t)xh->cmd_ring; /* replace if not identity */
    uint64_t crcr_low = (uint32_t)(cr_phys & 0xFFFFFFFF);
    uint64_t crcr_high = (uint32_t)(cr_phys >> 32);
    /* CRCR: low = pointer low, plus flags: RCS (Ring Cycle State) bit0 */
    uint32_t crcr_low_val = (uint32_t)(cr_phys & ~0xF) | (xh->cmd_ring_cycle & 1);
    /* Write CRCR low/high into operational registers */
    mmio_write32(xh->op_regs, XHCI_CRCR_LOW, crcr_low_val);
    mmio_write32(xh->op_regs, XHCI_CRCR_HIGH, (uint32_t)(cr_phys >> 32));
    memory_barrier();
    klog("xhci: command ring set at phys 0x%016llx\n", (unsigned long long)cr_phys);
    return true;
}

/* Simple function to push a TRB into the command ring (no wrap handling yet) */
static void xhci_push_command_trb(struct xhci *xh, struct xhci_trb *trb)
{
    static uint32_t cmd_idx = 0;
    uint32_t idx = cmd_idx % xh->cmd_ring_size;
    xh->cmd_ring[idx] = *trb;
    /* If we wrote the last TRB, set link TRB and toggle cycle — left as exercise */
    cmd_idx++;
}

/* Ring the doorbell for the command ring (doorbell 0 is for command ring) */
static inline void xhci_ring_doorbell(struct xhci *xh, uint32_t db_index, uint32_t value)
{
    /* Doorbell registers are 32-bit each starting at doorbell_array. */
    mmio_write32(xh->doorbell_array, db_index * 4, value);
}

/* -----------------------------
   Event Ring setup (very small)
   -----------------------------*/
static bool xhci_create_event_ring(struct xhci *xh)
{
    xh->event_ring_size = EVENT_RING_TRBS;
    xh->event_ring = (struct xhci_event_trb *)rmap(1);
    if (!xh->event_ring) return false;
    xh->event_ring_cycle = 1;
    xh->event_ring_consumer_index = 0;

    /* Build ERST (Event Ring Segment Table) - one segment only */
    xh->erst = rmap(1);
    if (!xh->erst) return false;
    /* ERST entry: 16 bytes -> {seg_addr_low, seg_addr_high, seg_size, reserved} */
    uint64_t seg_addr = (uint64_t)(uintptr_t)xh->event_ring;
    uint32_t *erst32 = (uint32_t *)xh->erst;
    erst32[0] = (uint32_t)(seg_addr & 0xFFFFFFFF);
    erst32[1] = (uint32_t)(seg_addr >> 32);
    erst32[2] = xh->event_ring_size;
    erst32[3] = 0;

    /* Write ERST and ERDP??? We'll set necessary runtime registers.
       For brevity we'll write the ERSTSZ and ERSTBA/ERDP runtime registers.
       These offsets vary by controller; in production read capability registers for runtime offset.
    */
    /* runtime offset pointers are not standardized here; for many controllers:
       runtime + 0x18 = ERSTSIZE, 0x20 = ERSTBA_LO, 0x24 = ERSTBA_HI, 0x28 = ERDP_LO, 0x2C = ERDP_HI
       We'll use these offsets as a heuristic. */
    size_t ERSTSZ = 0x18;
    size_t ERSTBA_LO = 0x20;
    size_t ERSTBA_HI = 0x24;
    size_t ERDP_LO = 0x28;
    size_t ERDP_HI = 0x2C;

    uint64_t erst_phys = (uint64_t)(uintptr_t)xh->erst;
    mmio_write32(xh->runtime_regs, ERSTSZ, 1); /* number of entries */
    mmio_write32(xh->runtime_regs, ERSTBA_LO, (uint32_t)(erst_phys & 0xFFFFFFFF));
    mmio_write32(xh->runtime_regs, ERSTBA_HI, (uint32_t)(erst_phys >> 32));
    mmio_write32(xh->runtime_regs, ERDP_LO, (uint32_t)((uint64_t)(uintptr_t)xh->event_ring & 0xFFFFFFFF));
    mmio_write32(xh->runtime_regs, ERDP_HI, (uint32_t)(((uint64_t)(uintptr_t)xh->event_ring) >> 32));

    klog("xhci: event ring created at %p (erst %p)\n", xh->event_ring, xh->erst);
    return true;
}

/* Poll event ring and handle events we know: Command Completion and Transfer Events */
static void xhci_poll_event_ring(struct xhci *xh)
{
    uint32_t idx = xh->event_ring_consumer_index;
    while (1) {
        struct xhci_event_trb *ev = &xh->event_ring[idx];
        uint32_t ctrl = ev->control;
        uint8_t cycle = (ctrl & 1);
        if (cycle != xh->event_ring_cycle) break; /* no new events */
        uint8_t type = (ctrl >> 10) & 0x3F;
        if (type == TRB_TYPE_COMMAND_COMPLETION) {
            klog("xhci: command completion event (status 0x%08x)\n", ev->status);
            /* In full driver match command completion by command TRB pointer etc */
        } else if (type == TRB_TYPE_TRANSFER_EVENT) {
            klog("xhci: transfer event (status 0x%08x)\n", ev->status);
            /* parse slot id / endpoint / bytes etc */
        } else {
            klog("xhci: event type %u\n", type);
        }
        /* Advance index */
        idx = (idx + 1) % xh->event_ring_size;
        if (idx == 0) xh->event_ring_cycle ^= 1;
        /* update ERDP to consumer index: write pointer to runtime register ERDP */
        uint64_t new_erdp = (uint64_t)(uintptr_t)&xh->event_ring[idx];
        mmio_write32(xh->runtime_regs, 0x28, (uint32_t)(new_erdp & 0xFFFFFFFF));
        mmio_write32(xh->runtime_regs, 0x2C, (uint32_t)(new_erdp >> 32));
    }
    xh->event_ring_consumer_index = idx;
}

/* -----------------------------
   xHCI Initialization sequence (simplified)
   -----------------------------*/
static bool xhci_reset_and_init(struct xhci *xh)
{
    /* Issue Host Controller Reset via USBCMD.HCRST */
    uint32_t usbcmd = mmio_read32(xh->op_regs, XHCI_USBCMD);
    mmio_write32(xh->op_regs, XHCI_USBCMD, usbcmd | USBCMD_HCRST);
    /* Wait for reset to clear */
    for (int i = 0; i < 100000; ++i) {
        uint32_t s = mmio_read32(xh->op_regs, XHCI_USBCMD);
        if (!(s & USBCMD_HCRST)) break;
        cpu_pause();
    }

    /* Enable interrupts? we'll leave interrupts disabled and poll event ring for simplicity */
    /* Create command ring and event ring */
    if (!xhci_create_command_ring(xh)) return false;
    if (!xhci_create_event_ring(xh)) return false;

    /* Start the controller by setting RUN bit */
    uint32_t new_usbcmd = mmio_read32(xh->op_regs, XHCI_USBCMD);
    new_usbcmd |= USBCMD_RUN_STOP;
    mmio_write32(xh->op_regs, XHCI_USBCMD, new_usbcmd);

    /* Wait for Host Controller to be Running: USBSTS.HCH=0 (Halted) cleared */
    for (int i = 0; i < 100000; ++i) {
        uint32_t sts = mmio_read32(xh->op_regs, XHCI_USBSTS);
        if (!(sts & USBSTS_HCH)) break;
        cpu_pause();
    }
    klog("xhci: controller started\n");
    return true;
}

/* -----------------------------
   Minimal Control Transfer (high-level)
   - Build Setup TRB, Data TRBs (if any), Status TRB, ring them on command ring as Command TRB(s)
   - Wait for command completion events in event ring (poll)
   NOTE: This is simplified: production driver uses transfer ring per endpoint and doorbells.
   -----------------------------*/

struct usb_setup_pkt {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} __attribute__((packed));

/* Build and queue a command TRB that performs a Device Request (Control Transfer) using xHCI TRB types */
static int xhci_control_transfer(struct xhci *xh, uint8_t slot_id,
                                 struct usb_setup_pkt *setup,
                                 void *data, uint16_t length, bool direction_in)
{
    /* Build Setup Stage TRB */
    struct xhci_trb setup_trb = {0};
    setup_trb.parameter = (uint64_t)(uintptr_t)setup; /* pointer to 8-byte setup */
    setup_trb.status = 0;
    setup_trb.control = (TRB_TYPE_SETUP_STAGE << 10) | TRB_CONTROL_IOC | (xh->cmd_ring_cycle & 1);

    /* Build Data Stage TRB if needed */
    struct xhci_trb data_trb = {0};
    if (length > 0) {
        data_trb.parameter = (uint64_t)(uintptr_t)data;
        data_trb.status = length << 0;
        data_trb.control = (TRB_TYPE_DATA_STAGE << 10) | (direction_in ? (1<<16) : 0) | TRB_CONTROL_IOC | (xh->cmd_ring_cycle & 1);
    }

    /* Build Status Stage TRB */
    struct xhci_trb status_trb = {0};
    status_trb.parameter = 0;
    status_trb.status = 0;
    status_trb.control = (TRB_TYPE_STATUS_STAGE << 10) | TRB_CONTROL_IOC | (xh->cmd_ring_cycle & 1);

    /* Push TRBs onto command ring (simple push) */
    xhci_push_command_trb(xh, &setup_trb);
    if (length > 0) xhci_push_command_trb(xh, &data_trb);
    xhci_push_command_trb(xh, &status_trb);

    /* Doorbell for Command Ring (Doorbell 0) to notify controller */
    xhci_ring_doorbell(xh, 0, 0);

    /* Poll event ring until completion or timeout */
    for (int t = 0; t < 100000; ++t) {
        xhci_poll_event_ring(xh);
        /* In a complete implementation, we'd examine events and match by TRB pointers and completion codes */
        cpu_pause();
    }

    /* For now return success placeholder */
    return 0;
}

/* -----------------------------
   Skeleton for slot/endpoint management (very simplified)
   -----------------------------*/
static int xhci_enable_slot(struct xhci *xh)
{
    /* Normally you issue an ENABLE_SLOT command TRB on command ring and wait for Command Completion event */
    /* Build Enable Slot Command TRB type = 9 (Enable Slot Command). We'll construct a command TRB and poll */
    struct xhci_trb cmd = {0};
    cmd.parameter = 0;
    cmd.status = 0;
    cmd.control = (9 << 10) | TRB_CONTROL_IOC | (xh->cmd_ring_cycle & 1); /* 9 = Enable Slot */
    xhci_push_command_trb(xh, &cmd);
    xhci_ring_doorbell(xh, 0, 0);
    /* Poll events to get slot id... */
    for (int i = 0; i < 100000; ++i) {
        xhci_poll_event_ring(xh);
        cpu_pause();
    }
    /* Return placeholder slot id */
    return 1;
}

/* -----------------------------
   Very small HID example: assume we have a device with an interrupt IN endpoint already configured
   We'll do an IN transfer by crafting a transfer TRB and ringing doorbell for that endpoint's slot/ep.
   This is left as a sketch to extend for your keyboard/mouse handling.
   -----------------------------*/
static void hid_poll_example(struct xhci *xh, uint8_t slot_id, uint8_t ep_index, void *buffer, size_t buf_len)
{
    /* Build a transfer TRB (Normal TRB) with pointer to buffer */
    struct xhci_trb trb = {0};
    trb.parameter = (uint64_t)(uintptr_t)buffer;
    trb.status = (uint32_t)buf_len;
    trb.control = (TRB_TYPE_NORMAL << 10) | TRB_CONTROL_IOC | (xh->cmd_ring_cycle & 1);
    /* In production you'd place this on the endpoint transfer ring and ring doorbell at db_index = slot*max_eps + ep */
    /* Here we'll do a simplified ring: push to command ring and ring doorbell as placeholder */
    xhci_push_command_trb(xh, &trb);
    xhci_ring_doorbell(xh, 1 /* db index heuristic */ , 0);
}

/* -----------------------------
   Public initializer entry
   -----------------------------*/
bool usb_core_init(void)
{
    struct pci_addr *p;
    if (!pci_find_xhci(&p)) {
        klog("usb: no xHCI controller found\n");
        return false;
    }

    g_xhci.pci = p;
    if (!xhci_map_mmio(&g_xhci)) {
        klog("usb: failed to map xHCI mmio\n");
        return false;
    }

    if (!xhci_reset_and_init(&g_xhci)) {
        klog("usb: failed to reset/init xHCI\n");
        return false;
    }

    klog("usb: xHCI core initialized successfully\n");
    return true;
}

/* Clean up (basic) */
void usb_core_shutdown(void)
{
    /* TODO: stop controller, free rings, unmap mmio */
    if (g_xhci.cmd_ring) runmap(g_xhci.cmd_ring, 1);
    if (g_xhci.event_ring) runmap(g_xhci.event_ring, 1);
    if (g_xhci.erst) runmap(g_xhci.erst, 1);
    if (g_xhci.mmio) mmio_unmap(g_xhci.mmio, 0x10000);
}
