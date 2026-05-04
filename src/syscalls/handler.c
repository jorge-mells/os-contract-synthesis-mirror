#include "handler.h"

void handle_syscall() {
    uint64_t syscall_num = current_running_pcb->context[17]; // a7 holds the "ID"
    
    switch (syscall_num) {
        case 64: { // SYS_WRITE
            uint64_t fd    = current_running_pcb->context[10]; // a0
            const char* buff     = (const char*)current_running_pcb->context[11]; // a1
            size_t count   = current_running_pcb->context[12]; // a2
            
            current_running_pcb->context[10] = write(fd, buff, count);
            current_running_pcb->pc = current_running_pcb->pc + 4;
            break;
        }

        case 93: { // SYS_EXIT
            // a0 (Register 10) is the exit status code
            int exit_status = (int)current_running_pcb->context[10];
            
            // Call your sys_exit function with the status
            sys_exit(exit_status);
            
            // sys_exit should handle the PCB cleanup and never return here
            break;
        }

        default:
            sys_exit(1);
            break;
    }
}