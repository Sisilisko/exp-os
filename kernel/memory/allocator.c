#include "common/types.h"
#include "common/memory.h"

void *mmap(/*size_t requested_amount*/) {
    return (void*)alloc_page();

}

void munmap(physaddr_t *freeable_ptr /* size_t regions*/ ) {
    free_page(*freeable_ptr);
    return;
}
