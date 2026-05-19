/**
 * @file scheduler.h
 * @brief programs loader and scheduler
 */
#ifndef SCHEDULER_H
#define SCHEDULER_H

#define MAX_PROCS 10
#define PROG_SIZE  4096
#define STACK_SIZE 4096
#define GUARD_SIZE 1024
#define BLOCK_SIZE (PROG_SIZE + STACK_SIZE + GUARD_SIZE)

#include "../platform/platform.h"


#ifndef __ASSEMBLER__
#include <stdint.h>
#include <stdbool.h>

/**
 * @name Process Control Block States
 * @anchor pcb_states
 * These constants define the lifecycle of a process within the scheduler.
 * @{
 */
#define PCB_STATE_UNUSED   0  /**< Slot is empty and available for a new process. */
#define PCB_STATE_USED     1  /**< Process is allocated but not yet initialized. */
#define PCB_STATE_SLEEPING 2  /**< Process is blocked waiting for I/O or a timer. */
#define PCB_STATE_READY    3  /**< Process is in the run-queue, waiting for CPU time. */
#define PCB_STATE_RUNNING  4  /**< Process is currently executing on a hart. */
#define PCB_STATE_ZOMBIE   5  /**< Process has exited but its exit code is unread. */
/** @} */

/**
 * @brief Process Control Block (PCB)
 * 
 * This structure maintains the entire execution context and metadata for a 
 * single process. It is used by the scheduler to save and restore states 
 * during context switches on the RISC-V harts.
 */
typedef struct pcb {
    /** @brief Remaining time units for the current scheduling quantum. */
    uint64_t time_slice;

    /** @brief Saved register state (x0-x31).
     *  Indices correspond to RISC-V general-purpose registers. 
     *  @note x0 is hardwired to zero but space is often kept for indexing consistency. */
    uint64_t context[32];

    /** @brief Unique Process Identifier. */
    uint64_t pid;

    /** @brief Current execution status (e.g., READY, RUNNING, ZOMBIE).
     *  @see pcb_states */
    uint64_t state;

    /** @brief Program Counter; the address of the next instruction to execute. */
    uint64_t pc;

    /** @brief The starting physical or virtual address of the process memory image. */
    uint64_t base_addr;
    
} pcb_t;

extern uintptr_t kernel_sp; // maybe replace these later with an actual pcb for the kernel
extern uintptr_t kernel_gp;

/**
 * @brief Global process table.
 * 
 * This table resides in the @c .bss section and is automatically zeroed out 
 * by the assembly bootloader in @c _start before the kernel starts.
 * 
 * @note Because of the boot-time zeroing, all processes initially have 
 *       a @ref PCB_STATE_UNUSED (0) state.
 */
extern pcb_t pcb_table[MAX_PROCS];

/**
 * @brief Pointer to the PCB currently executing on the hart.
 * 
 * During a context switch, this pointer is updated to reflect the new 
 * task.
 */
extern pcb_t *current_running_pcb;

/**
 * @brief Loads multiple programs from storage into the designated memory region.
 * 
 * This function iterates through the storage medium (starting at the Table 
 * of Contents), allocates memory blocks, and initializes the process metadata.
 * 
 * @param start_addr      The base physical address where loading begins.
 * @param space_allocated The total size (in bytes) available for all programs.
 * @param num_requested   The number of programs to load into RAM
 * 
 * @pre  The @ref next_prog_id must be initialized.
 * @post The @ref next_prog_id will be incremented accordingly.
 * 
 * @note **Side Effect:** This function directly modifies the global 
 *       @ref next_prog_id to ensure all programs will eventually be loaded off the disk.
 *       It thus loads the programs in order stored on disk.
 */
void load_programs(uint64_t start_addr, uint64_t space_allocated, uint8_t num_requested);

/**
 * @brief Main Scheduler Entry Point.
 * 
 * Orchestrates the transition between processes by performing three key roles:
 * 
 * 1. **Initial Boot:** If @ref current_running_pcb is @c NULL, it selects the 
 *    first process to start the system.
 * 2. **Cleanup:** Reclaims slots held by @ref PCB_STATE_ZOMBIE processes, 
 *    transitioning them back to @ref PCB_STATE_UNUSED.
 * 3. **Task Rotation:** Calls the selection logic to find the next @ref PCB_STATE_READY 
 *    task and updates the hardware execution context.
 * 
 */
void scheduler();
#endif

/**
 * @defgroup PCB_OFFSETS PCB Assembly Offsets
 * @brief Manual memory offsets for accessing @ref pcb_t members within assembly code.
 * 
 * @warning These must be kept in perfect sync with @ref pcb_t. If the struct 
 *          layout changes in C, these hardware offsets will break.
 * @{
 */

#define PCB_TIME_SLICE    0   /**< Offset for time_slice */

/** @name Register Context Offsets (x0-x31)
 *  Formula: 8 + (index * 8)
 *  @{ */
#define PCB_CONTEXT_BASE  8
#define PCB_REG_X0        8
#define PCB_REG_RA        16  
#define PCB_REG_SP        24  
#define PCB_REG_GP        32   /* x3 */
#define PCB_REG_TP        40   /* x4 */
#define PCB_REG_T0        48   /* x5 */
#define PCB_REG_T1        56   /* x6 */
#define PCB_REG_T2        64   /* x7 */
#define PCB_REG_S0        72   /* x8 / fp */
#define PCB_REG_S1        80   /* x9 */
#define PCB_REG_A0        88   /* x10 */
#define PCB_REG_A1        96   /* x11 */
#define PCB_REG_A2        104  /* x12 */
#define PCB_REG_A3        112  /* x13 */
#define PCB_REG_A4        120  /* x14 */
#define PCB_REG_A5        128  /* x15 */
#define PCB_REG_A6        136  /* x16 */
#define PCB_REG_A7        144  /* x17 */
#define PCB_REG_S2        152  /* x18 */
#define PCB_REG_S3        160  /* x19 */
#define PCB_REG_S4        168  /* x20 */
#define PCB_REG_S5        176  /* x21 */
#define PCB_REG_S6        184  /* x22 */
#define PCB_REG_S7        192  /* x23 */
#define PCB_REG_S8        200  /* x24 */
#define PCB_REG_S9        208  /* x25 */
#define PCB_REG_S10       216  /* x26 */
#define PCB_REG_S11       224  /* x27 */
#define PCB_REG_T3        232  /* x28 */
#define PCB_REG_T4        240  /* x29 */
#define PCB_REG_T5        248  /* x30 */
#define PCB_REG_T6        256 
/** @} */

/** @name Metadata Offsets
 *  @{ */
#define PCB_PID           264 /**< Offset for process ID */
#define PCB_STATE         272 /**< Offset for pcb_state */
#define PCB_PC            280 /**< Offset for program counter (mepc) */
#define PCB_BASE_ADDR     288 /**< Offset for process base memory address */
/** @} */

/** @} */ // end of PCB_OFFSETS

#endif