#include "traps.h"
void setup_traps(uintptr_t mstatus_mask, uintptr_t mie_mask, uintptr_t trap_entry) {
    // 2. Setup other important M-mode registers
    // We use inline assembly to write to the Control and Status Registers
    asm volatile (
        "csrw mtvec, %0\n\t"
        "csrs mie, %1\n\t"
        "csrs mstatus, %2\n\t"
        : 
        : "r"(trap_entry), "r"(mie_mask), "r"(mstatus_mask)
    );
}

__attribute__((noreturn)) 
void run_first_program(void) {
    // 1. Timer Logic - Pure C
    uintptr_t mstatus_val;
    asm volatile("csrr %0, mstatus" : "=r"(mstatus_val));

    if (mstatus_val & MIE_MTIE) {
        *(volatile uintptr_t*)CLINT_MTIMECMP = *(volatile uintptr_t*)CLINT_MTIME + current_running_pcb->time_slice;
    }

    // 2. The Handover
    // We use "memory" constraints so the compiler handles the addresses of 
    // kernel_sp, kernel_gp, and the PCB struct members for us.
    asm volatile (
        // --- SAVE KERNEL CONTEXT ---
        STR(SREG) " sp, %3\n\t"         // Save current SP to kernel_sp variable
        STR(SREG) " gp, %4\n\t"         // Save current GP to kernel_gp variable

        // --- SETUP FOR USER MODE ---
        "csrw mepc, %0\n\t"     // Set the jump target (User PC)
        "mv sp, %1\n\t"         // Switch to User Stack
        "mv gp, %2\n\t"         // Switch to User GP
        
        "mret"                  // Final drop to User Mode
        : 
        : "r"(current_running_pcb->pc), 
          "r"(current_running_pcb->context[2]), //sp
          "r"(current_running_pcb->context[3]), // gp
          "m"(kernel_sp),       // %3: The memory location of kernel_sp
          "m"(kernel_gp)        // %4: The memory location of kernel_gp
        : "memory"
    );
    __builtin_unreachable();
}