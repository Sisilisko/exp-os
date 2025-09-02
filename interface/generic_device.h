#ifndef GEN_DEV_H
#define GEN_DEV_H

#include "types.h"

typedef enum DevType {
    I, // Input Device
    O,  // Output device
    IO, // I & O capable
    WNET, // Wireless networking
    ETH // Ethernet/Wired Networking
} EDevType;

typedef struct dev_type
{
    EDevType type;
    bool is_usable;
    uint32_t index;
} device_t;


#endif