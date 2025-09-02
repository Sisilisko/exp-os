#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "context.h"
#include "types.h"
#define MAX_AMOUNT_OF_PROCESSES 1000

typedef enum { KERNEL, USER, UI, DAEMON } EProcType;



typedef struct Process
{
    int32_t pd; // This is Process Descriptor (pd) Not anything else!
    EProcType type;
    uint64_t wait_time;
    bool is_running;
    char *executable; // TODO: Implement file system so we can finally execute someo... Something :P
} process_t;

void jump_to();
void return_back();

process_t *evaluate_processes();

process_t *create_process(EProcType process_type, int argc, char **argv);

void terminate_process(process_t *terminatable_process);



#endif