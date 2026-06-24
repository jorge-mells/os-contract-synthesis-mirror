/**
 * @file syscalls.h
 * @brief various functions to handle various syscalls
 */
#ifndef SYSCALLS_H
#define SYSCALLS_H


#ifndef __ASSEMBLER__
#include <stdint.h>
#include <stddef.h>
#include "../process-mgmt/scheduler.h"

/**
 * @brief System call to write data to a file descriptor.
 * 
 * @param fd    The file descriptor to write to (e.g., 1 for stdout).
 * @param buff  Pointer to the user-space buffer containing the data.
 * @param count The number of bytes to write.
 * @return      The number of bytes successfully written.
 */
uintptr_t write(uintptr_t fd, const char* buff, size_t count);

/**
 * @brief Terminates the current process.
 * 
 * Sets the @ref current_running_pcb state to @ref PCB_STATE_UNUSED and 
 * records the exit status. This function triggers an immediate 
 * call to the @ref scheduler to switch to a new task.
 * 
 * @param status The exit code returned to the parent or kernel.
 */
void sys_exit(uintptr_t status);

/* yield control back to the scheduler */
uintptr_t sched_yield(void);

/* return the lower 32 bits of mtime */
uint32_t custom_get_mtime_lower();
#endif

#endif