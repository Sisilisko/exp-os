#include "scheduler.h"

process_t simultaenous_processes[MAX_AMOUNT_OF_PROCESSES];

uint32_t process_amount = 0;

process_t *evaluate_processes() {
    process_t *process_pointer = simultaenous_processes;
    

}

process_t *create_process(EProcType process_type, int argc, char **argv) {
    process_t *proc;
    proc->type = process_type;
    proc->wait_time = 0;
    proc->is_running = false;
    
    process_amount++;
    
    
    

}