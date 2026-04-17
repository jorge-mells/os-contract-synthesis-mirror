#ifndef INTERRUPTS_H
#define INTERRUPTS_H
#include <stdint.h>

#define MSTATUS_MIE (1 << 3) // Global machine interrupt enable
#define MIE_MEIE (1 << 11) // Machine External Interrupt Enable
#define MIE_MTIE (1 << 7)  // Machine Timer Interrupt Enable
#define MIE_MSIE (1 << 3)  // Machine Software Interrupt Enable

void setup_interrupts(unsigned long mstatus_val, unsigned long mie_val, uintptr_t trap_entry);

void main_trap_handler();

void reset_interrupts();

#endif