/**
 * @file uart.h
 * @brief Serial Communication Driver for RISC-V UART.
 * 
 * This header provides the interface for the Universal Asynchronous 
 * Receiver-Transmitter (UART), typically a 16550A-compatible controller. 
 * It also provides several higher level utility functions for console output
 * and input.
 */
#ifndef UART_H
#define UART_H

#include <stddef.h>
#include <stdarg.h>
#include "../platform/platform.h"

void uart_init(size_t);
int kputchar(int);
int kputs(const char *);
void kvprintf(const char *, va_list);
void kprintf(const char *, ...);

#endif
