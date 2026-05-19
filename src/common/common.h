/**
 * @file common.h
 * @brief Common utility functions and standard type definitions.
 */

#ifndef COMMON_H
#define COMMON_H
#include <stdint.h>
#include "../platform/platform.h"
#include "../uart/uart.h"

/**
 * @brief Converts a lowercase character to uppercase.
 * 
 * @param c The character to convert.
 * @return The uppercase version of @p c if it is lowercase; otherwise, returns @p c.
 */
int toupper(int c);

/**
 * @brief Halts the kernel execution due to an unrecoverable error.
 * 
 * Prints a formatted error message to the console and enters an infinite loop.
 * 
 * @param format The format string (printf-style).
 * @param ... Additional arguments for the format string.
 * @note This function is the "Point of No Return" for the kernel.
 */
void panic(const char *format, ...);

/**
 * @brief main kernel logger
 */
void klog(const char *format, ...);

/**
 * @name PMP Configuration Flags
 * Permission bits for the RISC-V Physical Memory Protection (PMP) entries.
 * These are used to restrict User-mode (U-mode) access to specific RAM regions.
 * @{
 */

/** @brief Grants Read permission. */
#define PMP_READ    (1)

/** @brief Grants Write permission. */
#define PMP_WRITE   (1 << 1)

/** @brief Grants Execute permission (required for code segments). */
#define PMP_EXECUTE (1 << 2)

/** 
 * @brief Locks the PMP entry. 
 * Once set, the entry cannot be modified until the CPU is reset. 
 * Also applies the restriction to Machine-mode if set.
 */
#define PMP_LOCKED  (1 << 7)

/** @} */

/**
 * @brief PMP Address Matching Modes.
 * 
 * Determines how the PMP region boundaries are calculated.
 */
typedef enum {
    /** @brief Disabled: This PMP entry is inactive. */
    PMP_OFF   = (0 << 3),

    /** @brief Top of Range: The region spans from the previous PMP entry to this one. */
    PMP_TOR   = (1 << 3),

    /** @brief Naturally Aligned 4-byte: Matches exactly one 4-byte region. */
    PMP_NA4   = (2 << 3),

    /** @brief Naturally Aligned Power-of-Two: Matches a power-of-two region (>= 8 bytes). */
    PMP_NAPOT = (3 << 3)
} pmp_mode_t;

/**
 * @brief Container for PMP register values.
 * 
 * Stores the calculated configuration mode and the physical addresses 
 * to be written into the @c pmpaddr registers.
 */
typedef struct {
  pmp_mode_t mode;   /**< The matching mode (e.g., TOR, NAPOT). */
  uintptr_t pmpaddr0; /**< Starting boundary or base address. */
  uintptr_t pmpaddr1; /**< Ending boundary (primarily for TOR mode). */
} pmpinfo_t;

/**
 * @brief Calculates the correct register bit-patterns for a memory range.
 * 
 * Translates a start and end address into the specific encoding required 
 * by the chosen @ref pmp_mode_t. 
 * 
 * @param mode       The desired matching mode.
 * @param start_addr The beginning of the protected region.
 * @param end_addr   The end of the protected region.
 * @return A populated @ref pmpinfo_t structure ready for hardware application.
 */
pmpinfo_t pmp_compute_encoding(pmp_mode_t mode, uintptr_t start_addr, uintptr_t end_addr);

/**
 * @brief Low-level hardware interface to write a PMP configuration byte.
 * 
 * Writes the 8-bit configuration (permissions + mode) into the correct 
 * slot within the @c pmpcfg CSRs.
 * 
 * @param index    The PMP entry index (0-15/63 depending on implementation).
 * @param cfg_byte The combined permission and mode bits.
 */
void set_pmp_cfg(uint8_t index, uint8_t cfg_byte);

/**
 * @brief Direct hardware write to a PMP address register.
 * 
 * Uses inline assembly to write an address (shifted) to the 
 * specified CSR. 
 * @param index The CSR index (e.g., 0 for pmpaddr0).
 * @param addr  The encoded address value.
 * 
 * @warning @p index must be a constant!!
 */
#define SET_PMP_ADDR(index, addr)\
    asm volatile("csrw pmpaddr" #index ", %0" : : "r"(addr));\

/**
 * @brief Configures a single PMP entry using NAPOT mode.
 * 
 * NAPOT (Naturally Aligned Power-of-Two) uses one register to define 
 * a power-of-two sized region.
 * 
 * @param reg_idx    The index of the PMP register to use.
 * @param pmpcfg_val The permission flags (Read/Write/Execute).
 * @param start      The start of the memory region.
 * @param end        The end of the memory region.
 * 
 * @warning @p reg_idx must be a constant!!
 * @see @ref SET_PMP_ADDR
 */
#define PMP_SET_NAPOT(reg_idx, pmpcfg_val, start, end) do { \
    uintptr_t _s = (uintptr_t)(start); \
    uintptr_t _e = (uintptr_t)(end); \
    pmpinfo_t _vals = pmp_compute_encoding(PMP_NAPOT, (_s), (_e)); \
    SET_PMP_ADDR(reg_idx, _vals.pmpaddr0); \
    set_pmp_cfg(reg_idx, (pmpcfg_val) | PMP_NAPOT); \
} while(0)

/**
 * @brief Configures a PMP region using TOR mode.
 * 
 * TOR (Top of Range) uses two registers to create an arbitrary memory 
 * range: [pmpaddr(n-1), pmpaddr(n)].
 * 
 * @param reg_floor  The index of the base register (usually n-1).
 * @param reg_top    The index of the top register (usually n).
 * @param pmpcfg_val The permission flags for the region.
 * @param start      The physical start address.
 * @param end        The physical end address.
 * 
 * @warning @p reg_floor and @p reg_top must be constants!!
 * @see @ref SET_PMP_ADDR
 */
#define PMP_SET_TOR(reg_floor, reg_top, pmpcfg_val, start, end) do { \
    uintptr_t _s = (uintptr_t)(start); \
    uintptr_t _e = (uintptr_t)(end); \
    pmpinfo_t _vals = pmp_compute_encoding(PMP_TOR, (_s), (_e)); \
    SET_PMP_ADDR(reg_floor, _vals.pmpaddr0); \
    SET_PMP_ADDR(reg_top, _vals.pmpaddr1); \
    set_pmp_cfg(reg_top, (pmpcfg_val) | PMP_TOR); \
} while (0)

#endif
