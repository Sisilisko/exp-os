#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

#define PAGE_SIZE 4096
#define BITMAP_WORD uint64_t

typedef uintptr_t physaddr_t;

// Memory Syscalls:
void *mmap(/*size_t requested_amount*/);
void munmap(physaddr_t *freeable_ptr /* size_t regions*/);

// __placeholder_allocator__: (Paging is only allocation way, for now...):
physaddr_t alloc_page(void);
// __placeholder_deallocator__: (^):
void free_page(physaddr_t paddr);

// Memory Utilities:

void memset(void *_dst, int val, size_t len);
void *memcpy(void *restrict dstptr, const void *restrict srcptr, size_t size);
void *memmove(void *dstptr, const void *srcptr, size_t size);
int memcmp(const void *aptr, const void *bptr, size_t size);


#endif