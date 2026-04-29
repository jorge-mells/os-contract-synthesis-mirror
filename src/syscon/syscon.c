#include <stdint.h>
#include "syscon.h"
#include "../uart/uart.h"
#include "../traps/traps.h"

void poweroff(void) {
  kputs("Poweroff requested");
  *(uint32_t *)SYSCON_ADDR = 0x5555;
}

void reboot(void) {
  kputs("Reboot requested");
  *(uint32_t *)SYSCON_ADDR = 0x7777;
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
