#include "uart/uart.h"
#include "syscon/syscon.h"
#include "common/common.h"
#include "process-mgmt/scheduler.h"
#include "traps/traps.h"

#define ARCH "RISC-V"
#define MODE 'M'
#define KERNEL_DATA_START 0x80000000
#define KERNEL_STACK_START 0x80100000 // based on linker script; note that stack grows downward

void kmain(void) {
  uart_init(UART_ADDR);

  kprintf("Hello %s World!\n", ARCH);
  kprintf("We are in %c-mode!\n", MODE);
  // uintptr_t value;
  // asm volatile ("csrr %0, pmpaddr0" : "=r"(value) : : "memory");
  // kprintf("Full pmpcfg0: 0x%p\n", value);
  setup_memory_protection();
  uint64_t first_program_pc = dispatch_programs();
  setup_traps(MSTATUS_MPP_U | MSTATUS_MPIE, MIE_MTIE, (uintptr_t)main_trap_handler, first_program_pc);
  cleanup_and_exit();
}


