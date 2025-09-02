#ifndef TIMING_H
#define TIMING_H
#include "types.h"

void pit_init(uint32_t frequency);
static inline void outb(uint16_t port, uint8_t val);

#endif