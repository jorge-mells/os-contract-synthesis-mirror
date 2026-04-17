#include "uart/uart.h"
#include "syscon/syscon.h"
#include "common/common.h"
#include "interrupts/interrupts.h"

#define ARCH "RISC-V"
#define MODE 'M'

void kmain(void) {
  uart_init(UART_ADDR);

  kprintf("Hello %s World!\n", ARCH);
  kprintf("We are in %c-mode!\n", MODE);
  setup_interrupts(MSTATUS_MIE, MIE_MSIE, (uintptr_t)main_trap_handler);
  // interrupt test: doesn't work for functions though
  asm volatile("ebreak");
  reset_interrupts();
  poweroff();
}

