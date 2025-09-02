#ifndef MODAPI_H
#define MODAPI_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


// What is this header file for?
// This header file is an API for interfacing directly with the "modloader". 

// IMPORTANT:
// kernapi.h is not meant for making modules, it is for extending the core kernel's functionalities.
// For making modules: use modapi.h aka "this file".

// Now starts the "API part itself":

// 0. module_init/exit function pointers:
typedef int (*mod_init_t)(void);
typedef int (*mod_exit_t)(void);

// 1. Module oriented functions and data:

// a: Datastructures/Defines for Modules:
typedef struct kernel_symbol
{
    const char *name;
    void *addr;
} ksym_t;

typedef struct ModDesc
{
    const char *name;
    const char *version;
    mod_init_t init; // Optional!
    mod_exit_t exit; // Optional!
    const ksym_t *exports; /* Terminated by {NULL,0}*/
} mod_desc_t;

// b: Module loading/unloading functions:
bool insmod(char *mod_id);

void rmmod(char *mod_id);

#define MOD_DESCRIPTOR(desc) \
    __attribute__((section(".moddesc"))) \
    const mod_desc_t desc


#endif