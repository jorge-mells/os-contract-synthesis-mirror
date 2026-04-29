#ifndef HANDLER_H
#define HANDLER_H

#ifndef __ASSEMBLER__
#include <stdint.h>
#include <stddef.h>
void handle_syscall(uint64_t* user_regs, uint64_t* pc);
#endif

#endif 