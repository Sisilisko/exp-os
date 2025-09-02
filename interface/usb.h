#ifndef USB_H
#define USB_H

#include "types.h"
#include "pci.h"

bool usb_core_init(void);
void usb_core_shutdown(void);

uint32_t register_usb(void);

#endif