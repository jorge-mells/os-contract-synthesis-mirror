#include "uart/uart.h"
#include "syscon/syscon.h"
#include "common/common.h"
#include "traps/scheduler.h"
#include "traps/interrupts.h"

#define ARCH "RISC-V"
#define MODE 'M'

void test() {
  asm volatile("ebreak");
}

void user_program_dummy() {
    uint64_t count = 0;
    while (1) {
        count++; // Just keeping the CPU busy
    }
}

pcb_t *current_running_pcb;

void kmain(void) {
  uart_init(UART_ADDR);

  kprintf("Hello %s World!\n", ARCH);
  kprintf("We are in %c-mode!\n", MODE);
  static pcb_t first_proc;
  first_proc.time_slice = INTERVAL;
  current_running_pcb = &first_proc;
  setup_traps(MSTATUS_MIE, MIE_MTIE, (uintptr_t)main_trap_handler);
  user_program_dummy();
  setup_traps(0, 0, 0);
  poweroff();
}

