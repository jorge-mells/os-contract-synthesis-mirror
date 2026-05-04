#include "syscalls.h"
#include "../uart/uart.h"
#include "../syscon/syscon.h"
#include "../process-mgmt/scheduler.h"
uint64_t write(uint64_t fd, const char* buff, size_t count) {

    if (fd == 1) { // stdout
        kprintf("User Code: ");
        for (size_t i = 0; i < count; i++) {
            kputchar(buff[i]);
        }
        // Convention: write returns number of bytes written in a0
        return count; 
    }
    return -1;
}

void sys_exit(uint64_t status) {
    // 1. Log the exit for debugging (optional)
    uint64_t pid = current_running_pcb->pid;
    kprintf("\n[Process %u Exited with status %u]\n", pid, status);
    current_running_pcb->state = PCB_STATE_UNUSED;
    scheduler();
}