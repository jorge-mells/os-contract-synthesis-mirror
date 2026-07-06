/**
 * @file platform.h
 * @brief Hardware Memory Map Definitions.
 *
 * This file defines the base addresses for memory-mapped I/O (MMIO)
 * peripherals. These addresses are platform-specific and correspond
 * to the hardware's address decoding logic.
 */

#ifndef PLATFORM_H
#define PLATFORM_H

#define PLATFORM_IBEX 1
#define PLATFORM_CVA6 2
#define PLATFORM_QEMU 3

#if PLATFORM_ID == PLATFORM_IBEX
#include "platform_ibex.h"
#elif PLATFORM_ID == PLATFORM_CVA6
#include "platform_cva6.h"
#elif PLATFORM_ID == PLATFORM_QEMU
#include "platform_qemu.h"
#else
#error "Unknown or undefined PLATFORM_ID"
#endif

/**
 * @name MMIO Settings
 * These defines describe the memory layout for I/O devices.
 * Currently, it expects devices to follow the VirtIO protocol
 * @{
 */

/** @brief Base address of the first peripheral. */
#define DISK_START ((uintptr_t)_disk_start)
/** @brief End address of disk if needed(for disks in ram) */
#define DISK_END ((uintptr_t)_disk_end)

/** @} */

/**
 * @brief Use this flag to turn on debug printing
 */
#define LOG_ENABLED 0

#if __riscv_xlen == 64
#define LREG ld
#define SREG sd
#define REGBYTES 8
#else
#define LREG lw
#define SREG sw
#define REGBYTES 4
#endif

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif /* PLATFORM_H */
