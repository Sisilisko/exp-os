#ifndef CONTEXT_H
#define CONTEXT_H

#include "common/types.h"

typedef struct Context
{
    uint64_t rsp;
    uint64_t rbx;
    uint64_t rbp;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    void (*rip)(void);
} ctx_t;


void save_context(ctx_t *ctx, void(*rip)(void));
void restore_context(ctx_t *ctx);


#endif