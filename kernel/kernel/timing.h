#ifndef TIMING_H
#define TIMING_H
#include "common/types.h"

void outbyte(uint16_t port, uint8_t val);
void pit_init(uint32_t frequency);

#endif