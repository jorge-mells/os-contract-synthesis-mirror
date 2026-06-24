#include <stdarg.h>
#include "common.h"

int toupper(int c) {
  return 'a' <= c && c <= 'z' ? c + 'A' - 'a' : c;
}

void panic(const char *format, ...) {
  kputs("Kernel panic!");
  kputs("Reason:");
  va_list arg;
  va_start(arg, format);
  kvprintf(format, arg);
  va_end(arg);
  asm volatile ("wfi");
}

#if LOG_ENABLED
    void klog_impl(const char *format, ...) {
        va_list arg;
        va_start(arg, format);
        kvprintf(format, arg);
        va_end(arg);
    }
#endif


pmpinfo_t pmp_compute_encoding(pmp_mode_t mode, uintptr_t start_addr, uintptr_t end_addr) {
  // check stuff satisfied for napot and if not just panic
  // conversion code
  pmpinfo_t info;
  info.mode = mode;
  info.pmpaddr0 = 0;
  info.pmpaddr1 = 0;

  uintptr_t size = end_addr - start_addr;

  switch (mode) {
      case PMP_NAPOT:
          // Safety: Size must be power of 2 and >= 8 bytes
          if (size < 8 || (size & (size - 1)) != 0) {
              panic("PMP_NAPOT size must be power of 2 and >= 8 bytes");
          }
          // Safety: Start address must be multiple of size
          if (start_addr % size != 0) {
              panic("PMP_NAPOT start address must be a multiple of size");
          }
          
          info.pmpaddr0 = (start_addr >> 2) | ((size >> 3) - 1);
          break;

      case PMP_TOR:
          info.pmpaddr0 = start_addr >> 2;
          info.pmpaddr1 = end_addr >> 2;
          break;

      case PMP_NA4:
          if (size != 4) panic("PMP NA4 invalid");
          info.pmpaddr0 = start_addr >> 2;
          break;

      default:
          // Handle PMP_OFF or invalid modes
          info.pmpaddr0 = 0;
          break;
  }

  return info;
}

void set_pmp_cfg(uint8_t index, uint8_t cfg_byte) {
  uint8_t reg_idx = (index < 8) ? 0 : 2; // pmpcfg0 or pmpcfg2
  uint8_t shift = (index % 8) * 8;
  uintptr_t val;

  if (reg_idx == 0) {
      asm volatile("csrr %0, pmpcfg0" : "=r"(val));
      val = (val & ~(0xFFULL << shift)) | ((uintptr_t)cfg_byte << shift);
      asm volatile("csrw pmpcfg0, %0" : : "r"(val));
  } else {
      asm volatile("csrr %0, pmpcfg2" : "=r"(val));
      val = (val & ~(0xFFULL << shift)) | ((uintptr_t)cfg_byte << shift);
      asm volatile("csrw pmpcfg2, %0" : : "r"(val));
  }
}
