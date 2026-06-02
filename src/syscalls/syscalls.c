#include "syscalls.h"
#include "../uart/uart.h"
#include "../common/common.h"
#include "../syscon/syscon.h"
#include "../process-mgmt/scheduler.h"
uintptr_t write(uintptr_t fd, const char* buff, size_t count) {

    if (fd == 1) { // stdout
        klog("User Code: ");
        for (size_t i = 0; i < count; i++) {
            kputchar(buff[i]);
        }
        // Convention: write returns number of bytes written in a0
        return count; 
    }
    return -1;
}

void sys_exit(uintptr_t status) {
    // 1. Log the exit for debugging (optional)
    uintptr_t pid = current_running_pcb->pid;
    klog("\n[Process %u Exited with status %u]\n", pid, status);
    current_running_pcb->state = PCB_STATE_UNUSED;
    scheduler();
}