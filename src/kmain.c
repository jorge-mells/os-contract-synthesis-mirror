#include "uart/uart.h"
#include "process-mgmt/storage-driver.h"
#include "syscon/syscon.h"
#include "common/common.h"
#include "process-mgmt/scheduler.h"
#include "process-mgmt/loader.h"
#include "traps/traps.h"
#include "heap-mgmt/kmalloc.h"

#define ARCH "RISC-V"
#define MODE 'M'

// NOTE: there are still lots of magic numbers littered in this codebase. The code is also quite messy but should hopefully be not too hard to follow
void kmain(void) {
  uart_init(UART_ADDR);

  kprintf("Hello %s World!\n", ARCH);
  kprintf("We are in %c-mode!\n", MODE);
  // I use mpie instead of mie since mret sets mie as what was in mpie and i don't want interrupts triggered until i mret.
  setup_traps(MSTATUS_MPP_U | MSTATUS_MPIE, MIE_MTIE, (uintptr_t)main_trap_handler);
  init_disk();
  // loads information about programs on the disk to be able to properly eventually load programs
  // it's done here so we only do it once forever
  init_toc();
  // 10000 is simply a random large number 0x82000000 is chosen since the kernel code + everything else ends at 0x81000000(see linker script)
  load_programs(0x82000000, MAX_PROCS * BLOCK_SIZE, 10000);
  scheduler();
  run_first_program();

  // we'll technically never get back here but in case we do, we can simply cleanly exit
  cleanup_and_exit();
}
