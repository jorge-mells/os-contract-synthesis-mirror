#include "uart/uart.h"
#include "syscon/syscon.h"
#include "common/common.h"
#include "process-mgmt/scheduler.h"
#include "traps/traps.h"

#define ARCH "RISC-V"
#define MODE 'M'

void kmain(void) {
  uart_init(UART_ADDR);

  kprintf("Hello %s World!\n", ARCH);
  kprintf("We are in %c-mode!\n", MODE);
  uint64_t first_program_pc = dispatch_programs();
  setup_traps(MSTATUS_MIE, MIE_MTIE, (uintptr_t)main_trap_handler, first_program_pc);
  cleanup_and_exit();
}


