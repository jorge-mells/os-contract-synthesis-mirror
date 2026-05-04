#include <stdint.h>
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
// Force the start to the next 4KB boundary so that user code doesn't need to be concerned about alignment
#define USER_SPACE_START  (((uintptr_t)__stack_top + 4095) & ~4095)

// NOTE: there are still lots of magic numbers littered in this codebase. The code is also quite messy but should hopefully be not too hard to follow
void kmain(void) {
  uart_init(UART_ADDR);

  kprintf("Hello %s World!\n", ARCH);
  kprintf("We are in %c-mode!\n", MODE);
  setup_traps(MSTATUS_MPP_U | MSTATUS_MPIE, MIE_MTIE, (uintptr_t)main_trap_handler);
  init_disk();
  load_prg_info();
  load_programs(USER_SPACE_START, MAX_PROCS * BLOCK_SIZE, total_prog_count);
  scheduler();
  run_first_program();

  // we'll technically never get back here but in case we do, we can simply cleanly exit
  cleanup_and_exit();
}
