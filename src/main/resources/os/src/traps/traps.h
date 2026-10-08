/**
 * @file traps.h
 * @brief interrupt and exception handlers
 */

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

// interrupt and exception bitmasks
/** @name MSTATUS Register Fields
 *  Bitmask definitions for the Machine Status Register.
 *  @{ */
#define MSTATUS_MIE  (1 << 3)  /**< Machine Interrupt Enable */
#define MSTATUS_MPIE (1 << 7)  /**< Previous Interrupt Enable */
#define MSTATUS_MPP_MASK (3 << 11) /**< Previous Privilege Mask */
#define MSTATUS_MPP_S    (1 << 11) /**< MPP Supervisor Mode */ 
#define MSTATUS_MPP_U    (0 << 11) /**< MPP User Mode */
/** @} */

/** @name MIE Register Fields
 *  Bitmask definitions for the Machine Interrupt Enable Register.
 *  @{ */
#define MIE_MSIE (1 << 3)  /**< Machine Software Interrupt Enable */
#define MIE_MTIE (1 << 7)  /**< Machine Timer Interrupt Enable */
#define MIE_MEIE (1 << 11) /**< Machine External Interrupt Enable */
/** @} */

#ifndef __ASSEMBLER__
#include <stdint.h>
#include "../process-mgmt/scheduler.h"
#include "../platform/platform.h"

extern pcb_t *current_running_pcb;
extern uintptr_t kernel_sp;
extern uintptr_t kernel_gp;

/**
 * @brief Sets the requisite registers so that traps and exceptions can be correctly processed
 * 
 * @param mstatus_mask The value to set mstatus to.
 * @param mie_mask The value to set mie to.
 * @param trap_entry The function which is run when traps are triggered
 * 
 * @note Find masks for the various bits of mstatus and mie in traps.h
 */
void setup_traps(uintptr_t mstatus_mask, uintptr_t mie_mask, uintptr_t trap_entry);

/**
 * @brief Low level assembly handler for hardware traps
 * 
 * This function is defined in trap-handler.S
 * 
 * @warning It must be 4-byte aligned since it is loaded into @c mtvec register.
 */
extern void main_trap_handler();

/**
 * @brief setup various registers and memory regions before calling mret
 * 
 * This function properly sets up mtime and mtimecmp, saves important kernel registers and calls mret
 * 
 * @note Attributes: naked, noreturn
 */
void run_first_program();

#endif

#endif