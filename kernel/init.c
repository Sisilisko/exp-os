#include "limine.h"
#include "printf.h"

#include "kernel/gdt.h"
#include "kernel/idt.h"
#include "kernel/timing.h"

#include "isched/scheduler.h"

static volatile LIMINE_BASE_REVISION(2);

void _start(void) {
    printf_("Tui");
    initiateGDT();
    set_idt();
    pit_init(1193182);
   // char *args1[2] = {"/system/foo", "--test"};
   // char *args2[2] = {"/system/bar", "-d"};
   // // Two hard coded processes (FOR TESTING ONLY!):
   // process_t *proc0 = create_process(KERNEL, 0, args1);
   // process_t *proc1 = create_process(USER, 0, args2);
   // // Then we evaluate them:
   // evaluate_loop();
   //     
   // proc0->
    
}