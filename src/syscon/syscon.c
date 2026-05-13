#include <stdint.h>
#include "syscon.h"
#include "../uart/uart.h"
#include "../traps/traps.h"

void poweroff(void) {
  kputs("Poweroff requested");
  SYSCON_ADDR = POWER_OFF_MAGIC_NUM;
}

void reboot(void) {
  kputs("Reboot requested");
  SYSCON_ADDR = POWER_RESTART_MAGIC_NUM;
}

void cleanup_and_exit(void) {
  setup_traps(0, 0, 0);
  poweroff();
  // 3. THE CRITICAL PART: If the hardware takes a few milliseconds 
  // to actually die, we MUST NOT do anything else.
  while(1) {
    asm volatile("wfi"); // Stay here in low power until the lights go out
  }
}
