#include "scheduler.h"
#include "../uart/uart.h"

pcb_t *current_running_pcb;

void random_prog() {
    while (1) {
    }
}

uint64_t dispatch_programs() {
    // load first 'program'
    // ensure start address is a usable one otherwise exception immediately triggered
    // WARN: pcb not fully fixed. fix it up including deciding where to store pcbs
    static pcb_t first_proc;
    first_proc.time_slice = INTERVAL;
    first_proc.context[31] = (uint64_t)&random_prog;
    kprintf("%p\n", &random_prog);
    current_running_pcb = &first_proc;
    return current_running_pcb->context[31];
}

void scheduler() {
};