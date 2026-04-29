#include "../uart/uart.h"
#include "handler.h"
#include "syscalls.h"

void handle_syscall(uint64_t* user_regs, uint64_t* pc) {
    uint64_t syscall_num = user_regs[17]; // a7 holds the "ID"
    
    switch (syscall_num) {
        case 64: { // SYS_WRITE
            uint64_t fd    = user_regs[10]; // a0
            const char* buff     = (const char*)user_regs[11]; // a1
            size_t count   = user_regs[12]; // a2
            
            user_regs[10] = write(fd, buff, count);
            *pc = *pc + 4;
            break;
        }

        case 93: { // SYS_EXIT
            // a0 (Register 10) is the exit status code
            int exit_status = (int)user_regs[10];
            
            // Call your sys_exit function with the status
            sys_exit(exit_status);
            
            // sys_exit should handle the PCB cleanup and never return here
            break;
        }

        default:
            // Return -1 for unknown syscalls
            user_regs[10] = -1;
            break;
    }
}