#ifndef MODAPI_H
#define MODAPI_H

#include "types.h"



// What is this header file for?
// This header file is an API for interfacing directly with the "modloader". 

// IMPORTANT:
// kernapi.h is not meant for making modules, it is for extending the core kernel's functionalities.
// For making modules: use modapi.h aka "this file".

// Now starts the "API part itself":

// 1. Module oriented functions and data:

// a: Datastructures/Defines for Modules:
#define MAXMODNAME 255
#define MAXMODPATH 255

typedef struct ModInfo
{
    char modName[MAXMODNAME];
    char modPath[MAXMODPATH];
    uint64_t modVersion;

    bool enabled;
    bool loaded;
} modinfo_t;

// b: Module loading/unloading functions:
bool insmod(char *mod_id);

void rmmod(char *mod_id);



#endif