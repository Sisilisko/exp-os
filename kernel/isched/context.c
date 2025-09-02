#include "types.h"
#include "isched/context.h"


void save_context(ctx_t *ctx, void (*rip)(void)) {
    asm volatile (
        "movq %%rsp, %0\n\t"
        "movq %%rbx, %1\n\t"
        "movq %%rbp, %2\n\t"
        "movq %%r12, %3\n\t"
        "movq %%r13, %4\n\t"
        "movq %%r14, %5\n\t"
        "movq %%r15, %6\n\t"
        : "=m"(ctx->rsp), "=m"(ctx->rbx), "=m"(ctx->rbp),
          "=m"(ctx->r12), "=m"(ctx->r13), "=m"(ctx->r14),
          "=m"(ctx->r15)
        :
        : "memory"
    );
    ctx->rip = rip;
}

void restore_context(ctx_t *ctx) {
    asm volatile (
        "movq %0, %%rsp\n\t"
        "movq %1, %%rbx\n\t"
        "movq %2, %%rbp\n\t"
        "movq %3, %%r12\n\t"
        "movq %4, %%r13\n\t"
        "movq %5, %%r14\n\t"
        "movq %6, %%r15\n\t"
        "jmp *%7\n\t"
        :
        : "m"(ctx->rsp), "m"(ctx->rbx), "m"(ctx->rbp),
          "m"(ctx->r12), "m"(ctx->r13), "m"(ctx->r14),
          "m"(ctx->r15), "m"(ctx->rip)
        : "memory"
    );
}