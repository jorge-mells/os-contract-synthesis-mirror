#ifndef SCHEDULER_H
#define SCHEDULER_H

#ifndef __ASSEMBLER__
#include <stdint.h>
#include <stdbool.h>
typedef struct pcb {
    /* OFFSET 0: Hardware Timer Data
       Guaranteed by C standard to be at the start of the struct. */
    uint64_t time_slice;

    // pc is at idx 31
    uint64_t context[32];

    uint32_t pid;
    uint32_t state;
    
} pcb_t;

extern pcb_t *current_running_pcb;

uint64_t dispatch_programs();
void scheduler();
#endif

#define INTERVAL 10000000 // cpu dependent

#endif