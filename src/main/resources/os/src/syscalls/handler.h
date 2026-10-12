/**
 * @file handler.h
 * @brief main handler for syscalls which dispatches them to various functions
 */
#ifndef HANDLER_H
#define HANDLER_H

#ifndef __ASSEMBLER__
#include <stdint.h>
#include <stddef.h>
#include "../uart/uart.h"
#include "syscalls.h"
#include "../process-mgmt/scheduler.h"
#include "../common/common.h"

/**
 * @brief Dispatcher for user requests (ecall).
 * 
 * Logic:
 * 1. Reads syscall ID from @c a7 and arguments from @c a0-a5.
 * 2. Executes kernel service.
 * 3. Saves return value to @c a0.
 * 4. **Important:** May have to increment the saved PC to skip some ecall instruction.
 */
void handle_syscall();
#endif

#endif 