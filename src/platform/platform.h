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

/** 
 * @name CLINT (Core Local Interruptor) Registers
 * The CLINT block provides per-hart interrupts for software and timers.
 * @{
 */

/** 
 * @brief Machine Mode Timer Compare Register.
 * Writing to this address allows the kernel to schedule the next timer interrupt.
 * When mtime >= mtimecmp, a timer interrupt is posted.
 */
#define CLINT_MTIMECMP 0x02004000

/** 
 * @brief Machine Mode Timer Register.
 * A real-time counter that increments at a fixed frequency provided by the 
 * hardware crystal oscillator.
 */
#define CLINT_MTIME    0x0200bff8

#define INTERVAL 1000000

/**
 * @brief System Controller (syscon) address.
 * 
 * Used for power management operations like poweroff and reboot.
 * This address is mapped to the 'test' device in the QEMU virt machine.
 */
#ifndef __ASSEMBLER__
#include <stdint.h>
extern volatile uintptr_t tohost;
extern volatile uintptr_t fromhost;
#endif
// #define SYSCON_ADDR *(volatile uint32_t *)0x100000UL
#define SYSCON_ADDR tohost
// #define POWER_OFF_MAGIC_NUM 0x5555
#define POWER_OFF_MAGIC_NUM 1
#define POWER_RESTART_MAGIC_NUM 0x7777

/** @} */

/**
 * @name MMIO Settings
 * These defines describe the memory layout for I/O devices.
 * Currently, it expects devices to follow the VirtIO protocol
 * @{
 */

/** @brief Base address of the first peripheral. */
#define DISK_START         0x80100000
/** @brief End address of disk if needed(for disks in ram) */
#define DISK_END           0x80200000

/** @} */

/**
 * @brief Base address of the Universal Asynchronous Receiver-Transmitter (UART).
 * 
 * This MMIO address points to the UART controller (typically 16550A compatible 
 * on QEMU virt), which handles serial communication. Writing to these registers 
 * sends data to the console, while reading from them retrieves keyboard input.
 */
#define UART_ADDR 0x10000000

/**
 * @brief Use this flag to turn on debug printing
 */
#define LOG_ENABLED 1

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