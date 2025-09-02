#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

void *rmap(size_t requested_amount);
void runmap(void *freeable_ptr, size_t regions);

#endif