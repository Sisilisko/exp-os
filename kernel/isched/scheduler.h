#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "types.h"
#define MAX_AMOUNT_OF_PROCESSES 1000


typedef enum { KERNEL, USER, UI, DAEMON } EProcType;

typedef struct Context
{
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rsp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
} ctx_t;

typedef struct Process
{
    uint32_t pd; // This is Process Descriptor (pd) Not anything else!
    EProcType type;
    uint64_t wait_time;
    bool is_running;
    char *executable; // TODO: Implement file system so we can finally execute someo... Something :P
} process_t;

process_t *evaluate_processes();

process_t *create_process(EProcType process_type, int argc, char **argv);

void terminate_process(process_t *terminatable_process);

void continue_process(process_t *target_process);


#endif