#include "limine.h"

#ifndef BOOTLOADER_H
#define BOOTLOADER_H

extern uint64_t kernel_text_start, kernel_text_end;
extern uint64_t kernel_rodata_start, kernel_rodata_end;
extern uint64_t kernel_data_start, kernel_data_end;
extern uint64_t kernel_start, kernel_end;

#endif