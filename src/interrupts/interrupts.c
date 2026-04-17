#include <stdint.h>
#include "interrupts.h"
#include "../uart/uart.h"

// NOTE: only non vectored mode is currently supported
void setup_interrupts(unsigned long mstatus_val, unsigned long mie_val, uintptr_t trap_entry) {
    asm volatile (
        "csrs mstatus, %0\n\t"
        "csrs mie, %1\n\t"
        "csrw mtvec, %2\n\t" 
        :
        : "r"(mstatus_val), "r"(mie_val), "r"(trap_entry)
    );
}

// NOTE: this still doesn't work if the next thing is a function strangely. I probably have to fix the mepc_val += 2 stuff or go
// to pure assembly
__attribute__((aligned(4)))
void main_trap_handler() {
    unsigned long mcause_val, mepc_val;
    asm volatile ("csrr %0, mcause" : "=r"(mcause_val));
    asm volatile ("csrr %0, mepc" : "=r"(mepc_val));

    // Check if it's an Exception (highest bit is 0)
    if ((long)mcause_val >= 0) {
        kprintf("Exception %u at %p. Skipping...\n", mcause_val, (void*)mepc_val);
        
        // it's 2 instead of 4 because of the stupid compiler. I have no idea how to put it off(I should probably use assembly)
        mepc_val += 2; 
        asm volatile (
            "csrw mepc, %0\n\t" 
            "mret\n\t"
            : 
            : "r"(mepc_val)
        );
    } else {
        kprintf("Interrupt %lu handled.\n", mcause_val & 0x7FFFFFFF);
        // Interrupts don't need mepc increments!
    }
}

void handle_timer_interrupts();

void handle_syscall();

void handle_exceptions();

void reset_interrupts() {
    asm volatile (
        "csrc mstatus, %0\n\t"
        "csrw mie, zero\n\t"
        "csrw mtvec, zero"
        : 
        : "r"(MSTATUS_MIE)
    );
};