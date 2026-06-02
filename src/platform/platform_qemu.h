/**
 * @file platform.h
 * @brief Hardware Memory Map Definitions.
 * 
 * This file defines the base addresses for memory-mapped I/O (MMIO) 
 * peripherals. These addresses are platform-specific and correspond 
 * to the hardware's address decoding logic.
 */

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
#define CLINT_MTIMECMP 0x02004000 // qemu & cva6

/** 
 * @brief Machine Mode Timer Register.
 * A real-time counter that increments at a fixed frequency provided by the 
 * hardware crystal oscillator.
 */
#define CLINT_MTIME    0x0200bff8 // qemu & cva6

#define INTERVAL 10000

/**
 * @brief System Controller (syscon) address.
 * 
 * Used for power management operations like poweroff and reboot.
 * This address is mapped to the 'test' device in the QEMU virt machine.
 */
#ifndef __ASSEMBLER__
#include <stdint.h>
extern char _disk_start[];
extern char _disk_end[];
#endif
#define SYSCON_ADDR *(volatile uint32_t *)0x100000UL // qemu
#define POWER_OFF_MAGIC_NUM 0x5555 // qemu
#define POWER_RESTART_MAGIC_NUM 0x7777 // qemu

/**
 * @brief Base address of the Universal Asynchronous Receiver-Transmitter (UART).
 * 
 * This MMIO address points to the UART controller (typically 16550A compatible 
 * on QEMU virt), which handles serial communication. Writing to these registers 
 * sends data to the console, while reading from them retrieves keyboard input.
 */
#define UART_ADDR 0x10000000 // qemu & cva6