#include "limine.h"
#include "serial.h"
#include "printf.h"

#include "kernel/gdt.h"
#include "kernel/idt.h"
#include "kernel/timing.h"

#include "isched/scheduler.h"

static volatile LIMINE_BASE_REVISION(2);

void _start(void) {

    initiateGDT();
    set_idt();
    pit_init(1193182);

    // Two hard coded processes (FOR TESTING ONLY!):
    process_t *proc1 = create_process(KERNEL, NULL, NULL);
    process_t *proc2 = create_process(USER, NULL, NULL);
    // Then we evaluate them:
    process_t *process_to_continue = evaluate_processes();
    // Then we shall continue the one with higher priority &| wait time:
    continue_process(process_to_continue);

    initiateSerial();



    
}