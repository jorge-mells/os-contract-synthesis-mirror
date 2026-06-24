#include "syscalls.h"
#include "../uart/uart.h"
#include "../common/common.h"
#include "../syscon/syscon.h"
#include "../process-mgmt/scheduler.h"
#include "../platform/platform.h"
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

uintptr_t sched_yield() {
    uintptr_t pid = current_running_pcb->pid;
    klog("\n[Process %u yielded control]\n", pid);
    scheduler();
    return 0;

}

uint32_t custom_get_mtime_lower() {
    // 1. Read the full 64-bit value to ensure atomicity/correctness
    uint64_t full_time = *(volatile uint64_t*)CLINT_MTIME;
    
    // 2. Safely cast the result to 32 bits
    return (uint32_t)(full_time & 0xFFFFFFFF);
}
