#include "common/memory.h"

void memset(void *_dst, int val, size_t len) {
  __asm__ __volatile__("pushf; cld; rep stosb; popf"
               :
               : "D"(_dst), "a"(val), "c"(len)
               : "memory");
}

void *memcpy(void *restrict dstptr, const void *restrict srcptr, size_t size) {
  __asm__ __volatile__("pushf; cld; rep movsb; popf"
               :
               : "S"(srcptr), "D"(dstptr), "c"(size)
               : "memory");
  return dstptr;
}

void *memmove(void *dstptr, const void *srcptr, size_t size) {
  unsigned char       *dst = (unsigned char *)dstptr;
  const unsigned char *src = (const unsigned char *)srcptr;
  if (dst < src) {
    for (size_t i = 0; i < size; i++)
      dst[i] = src[i];
  } else {
    for (size_t i = size; i != 0; i--)
      dst[i - 1] = src[i - 1];
  }
  return dstptr;
}

int memcmp(const void *aptr, const void *bptr, size_t size) {
  const unsigned char *a = (const unsigned char *)aptr;
  const unsigned char *b = (const unsigned char *)bptr;
  for (size_t i = 0; i < size; i++) {
    if (a[i] < b[i])
      return -1;
    else if (b[i] < a[i])
      return 1;
  }
  return 0;
}