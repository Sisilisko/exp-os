#include "timing.h"
#include "common/types.h"
// Ports
#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43

// PIT frequency
#define PIT_FREQUENCY 1193182

// Write a byte to an I/O port
void outbyte(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Setup PIT to generate IRQ0 at given frequency
void pit_init(uint32_t frequency) {
    uint32_t divisor = PIT_FREQUENCY / frequency;

    // Command byte:
    // Channel 0, Access mode = lobyte/hibyte, Mode 2 (rate generator), Binary
    outbyte(PIT_COMMAND, 0x36);

    // Send divisor low byte, then high byte
    outbyte(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outbyte(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));
}
