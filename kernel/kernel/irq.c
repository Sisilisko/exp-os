#include "timing.h"
#include "types.h"

volatile uint64_t ticks = 0;

void irq0_handler() {
    ticks++;
    outb(0x20, 0x20);
    return;
}