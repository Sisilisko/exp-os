#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifndef TYPES_H
#define TYPES_H


#define ERR_INDIRECT(errno) (errno##UL)
#define ERR(errno) (-ERR_INDIRECT(errno))

#define INT_MAX 2147483647

#define low_16(address) (uint16_t)((address) & 0xFFFF)
#define high_16(address) (uint16_t)(((address) >> 16) & 0xFFFF)

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define force_inline inline __attribute__((always_inline))

#define PRINTF_ALIAS_STANDARD_FUNCTION_NAMES_SOFT 1
#define PRINTF_ALIAS_STANDARD_FUNCTION_NAMES 1

#include "limine.h"
#include "printf.h"



#endif