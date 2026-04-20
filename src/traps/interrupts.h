#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#define MSTATUS_MIE (1 << 3) // Global machine interrupt enable
#define MIE_MEIE (1 << 11) // Machine External Interrupt Enable
#define MIE_MTIE (1 << 7)  // Machine Timer Interrupt Enable
#define MIE_MSIE (1 << 3)  // Machine Software Interrupt Enable

#ifndef __ASSEMBLER__
#include <stdint.h>
extern void setup_traps(uint64_t mstatus_mask, uint64_t mie_mask, uintptr_t trap_entry);

extern void main_trap_handler();
#endif

#endif