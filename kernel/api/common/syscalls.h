#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "types.h"

// File Management:

void fwrite();
void fread();

// Memory:

// Region MAP/MAP REGION
void *rmap();
// Region UNMAP/UNMAP REGION
void runmap();



// Devices:



#endif