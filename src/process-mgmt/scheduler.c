#include "loader.h"
#include "../common/common.h"
#include "../syscon/syscon.h"
#include "../common/common.h"
#include "scheduler.h"
#include "../uart/uart.h"

uint64_t kernel_sp, kernel_gp;
pcb_t pcb_table[MAX_PROCS];
pcb_t *current_running_pcb;

/**
 * @brief Global counter for assigning unique process identifiers and tracking which programs
 * have already been loaded.
 * 
 * This variable tracks the last assigned ID to ensure every loaded program 
 * receives a unique identifier. 
 * 
 * @note **Initial Value:** Set to @c -1 (underflow state) so that the first 
 *       program loaded increments this value to @c 0.
 * @warning This variable is a **critical dependency** for @ref load_programs; 
 *          if not initialized correctly, PID collisions could occur or some programs might be skipped.
 */
uint64_t next_prog_id = -1;

void load_programs(uint64_t start_addr, uint64_t space_allocated, uint8_t num_requested) {
    uint64_t total_progs_on_disk = total_prog_count;
    
    // 1. Check if we've already finished the disk
    // If next_prog_id was 5 and we have 5 progs, we're done.
    if (next_prog_id != -1 && (uint64_t)next_prog_id >= total_progs_on_disk) {
        return; 
    }

    // 2. Initialize our bookmark if this is the first ever call
    if (next_prog_id == -1) {
        next_prog_id = 0;
    }

    // 3. Determine how many we can actually load right now
    // Don't exceed what's left on disk, and don't exceed the request
    uint64_t remaining_on_disk = total_progs_on_disk - next_prog_id;
    uint64_t to_load = (num_requested < remaining_on_disk) ? num_requested : remaining_on_disk;

    struct toc_entry *entries = (struct toc_entry *)&toc_buffer[8];

    for (int i = 0; i < to_load; i++) {
        // Find the next empty PCB slot
        int pcb_idx = -1;
        for (int j = 0; j < MAX_PROCS; j++) {
            if (pcb_table[j].state == PCB_STATE_UNUSED) {
                pcb_idx = j;
                break;
            }
        }

        // If no PCB slots left, we can't load anymore
        if (pcb_idx == -1) break;

        // Use next_prog_id to get the right entry from the TOC
        int current_disk_idx = next_prog_id;
        
        // Calculate memory offset based on the PCB index to avoid collisions
        uint64_t block_base = start_addr + (pcb_idx * BLOCK_SIZE);
        
        // Safety check: is this within the space we were told we have?
        if ((pcb_idx + 1) * BLOCK_SIZE > space_allocated) {
            break; 
        }

        // 1. Load from disk
        uint64_t prog_sector = entries[current_disk_idx].start_sector;
        load_program(prog_sector, block_base);

        // 2. Setup PCB at the found index
        pcb_t *p = &pcb_table[pcb_idx];
        p->time_slice = INTERVAL;
        p->pid = pcb_idx; 
        p->state = PCB_STATE_READY;    // Ready state
        p->pc = block_base;       // Entry point
        
        // 3. Context / Registers
        for(int reg = 0; reg < 32; reg++) p->context[reg] = 0;

        // 4. Stack Pointer (x2/sp)
        p->context[2] = block_base + PROG_SIZE + STACK_SIZE;

        // set base_addr
        p->base_addr = block_base;
        // Increment our global disk bookmark
        next_prog_id++;
    }
}

void add_memory_protections() {
    // 1. Get the base address of the current process's memory
    uint64_t start = current_running_pcb->base_addr;

    // 2. Define the 'End' of the usable area (Program + Stack = 8KB)
    uint64_t usable_end = start + PROG_SIZE + STACK_SIZE;

    // 3. Define the 'End' of the protected block (including the Guard)
    uint64_t block_end = start + BLOCK_SIZE;
    // APPLY THE PROTECTION
    // Entry 0 (Floor) to Entry 1 (Top): RWX permissions for the program/stack
    PMP_SET_TOR(0, 1, (PMP_READ | PMP_WRITE | PMP_EXECUTE), start, usable_end);
    // Entry 1 (Floor) to Entry 2 (Top): NO permissions for the guard area
    // This will trigger an Exception 1/5/7 if the stack grows into it
    PMP_SET_TOR(1, 2, 0, usable_end, block_end);
}

void remove_memory_protections() {
    // Setting the config to 0 (PMP_OFF) disables the range check
    PMP_SET_TOR(0, 1, 0, 0, 0);
    PMP_SET_TOR(1, 2, 0, 0, 0);
}

/**
 * @brief Round-Robin Task Selector.
 * 
 * Cycles through the @ref pcb_table to pick the next task to execute.
 * 
 * @return **Execution Status:**
 *         - @c 1: Success. A runnable process was found and scheduled.
 *         - @c 0: Failure. No runnable processes exist; the CPU has no work to do.
 * 
 * @important The return value determines whether the kernel should resume 
 *            execution or exit.
 */
int simple_round_robin() {
    // 2. Find the next READY process using Round Robin
    // We start searching at the NEXT index after the current one
    int current_idx = (int)(current_running_pcb - &pcb_table[0]);
    int next_idx;

    for (int i = 1; i <= MAX_PROCS; i++) {
        next_idx = (current_idx + i) % MAX_PROCS;
        pcb_t *p = &pcb_table[next_idx];

        if (p->state == PCB_STATE_READY) {
            // Found a process to run!
            current_running_pcb = p;
            p->state = PCB_STATE_RUNNING;
            add_memory_protections();
            return 1;
        }
    }
    return 0;
}

void scheduler() {


    // If this was the first run, set the current running pcb
    if (current_running_pcb == NULL) {
        current_running_pcb = &pcb_table[0];
        add_memory_protections();
        klog("\nStarting with Process %u...\n", current_running_pcb->pid);
        return;
    }
    // 1. If the current slot just became UNUSED (process exited), 
    // try to load a new program from disk into that same memory hole.
    if (current_running_pcb->state == PCB_STATE_UNUSED) {
        load_programs(current_running_pcb->base_addr, BLOCK_SIZE, 1);
    }
    // set state of current pcb to ready
    if (current_running_pcb->state == PCB_STATE_RUNNING) {
        current_running_pcb->state = PCB_STATE_READY;
        remove_memory_protections();
    }

    int status = simple_round_robin();
    if (status) {
        klog("\nSwitching to Process %u...\n", current_running_pcb->pid);
        return;
    }

    // 3. If the loop finishes, it means we checked every single slot
    // and found nothing READY. Time to shut down.
    klog("All programs executed\n");
    cleanup_and_exit();
}