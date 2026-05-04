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

/**
 * @brief System Controller (syscon) address.
 * 
 * Used for power management operations like poweroff and reboot.
 * This address is mapped to the 'test' device in the QEMU virt machine.
 */
#define SYSCON_ADDR 0x100000UL

/** @} */

/**
 * @name MMIO Settings
 * These defines describe the memory layout for I/O devices.
 * Currently, it expects devices to follow the VirtIO protocol
 * @{
 */

/** @brief Base address of the first peripheral. */
#define MMIO_START         0x10001000

/** @brief Memory spacing between consecutive device register blocks (4KB). */
#define MMIO_STEP          0x1000

/** @} */

/**
 * @brief Base address of the Universal Asynchronous Receiver-Transmitter (UART).
 * 
 * This MMIO address points to the UART controller (typically 16550A compatible 
 * on QEMU virt), which handles serial communication. Writing to these registers 
 * sends data to the console, while reading from them retrieves keyboard input.
 */
#define UART_ADDR 0x10000000

#endif /* PLATFORM_H */