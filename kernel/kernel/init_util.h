#ifndef INIT_UTIL_H
#define INIT_UTIL_H

#include "types.h"

void memset(void *_dst, int val, size_t len);
void *memcpy(void *restrict dstptr, const void *restrict srcptr, size_t size);
void *memmove(void *dstptr, const void *srcptr, size_t size);
int memcmp(const void *aptr, const void *bptr, size_t size);

#endif