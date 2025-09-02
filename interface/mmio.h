#ifndef MMIO_H
#define MMIO_H

#include "types.h"

void *mmio_map_phys(uint64_t phys, size_t size);
void mmio_unmap(void *vaddr, size_t size);

/* Read/write mmio helpers (volatile) */
static inline uint32_t mmio_read32(void *base, size_t off) {
    volatile uint32_t *p = (volatile uint32_t *)((char*)base + off);
    return *p;
}
static inline void mmio_write32(void *base, size_t off, uint32_t v) {
    volatile uint32_t *p = (volatile uint32_t *)((char*)base + off);
    *p = v;
}
static inline uint64_t mmio_read64(void *base, size_t off) {
    volatile uint64_t *p = (volatile uint64_t *)((char*)base + off);
    return *p;
}
static inline void mmio_write64(void *base, size_t off, uint64_t v) {
    volatile uint64_t *p = (volatile uint64_t *)((char*)base + off);
    *p = v;
}

#endif