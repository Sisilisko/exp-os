#include "scheduler.h"

process_t simultaenous_processes[MAX_AMOUNT_OF_PROCESSES];

uint32_t process_amount = 0;

ctx_t jump_context; // The Context, where to jump.
ctx_t jump_back_context; // The Old Context, where to return.

bool interrupted = false;

void switch_context() {
    save_context(&jump_context, &&back_point);
    restore_context(&jump_back_context);

back_point:
    return;
}

void switch_back_context() {
    save_context(&jump_back_context, &&back_point);
    restore_context(&jump_context); 

back_point:
    return;
}


void evaluate_loop() {
    if (interrupted)
    {
    
       switch_context();
        
    }
    interrupted = false;
}

process_t *create_process(EProcType process_type /* int argc, char *argv[] */) {
    process_t *proc = NULL;
    proc->type = process_type;
    proc->wait_time = 0;
    proc->is_running = true;
    proc->pd = process_amount;
    process_amount++;   
    
    return proc;
}

void terminate_process(process_t *terminatable_process) {
    terminatable_process->is_running = false;
    terminatable_process->pd = -1;
    process_amount--;
    return;
}


