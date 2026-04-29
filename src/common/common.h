#ifndef COMMON_H
#define COMMON_H
#include <stdint.h>

int toupper(int);
void panic(const char *, ...);

/* PMP Register Assignments (Fixed for Macros) */
#define PMP_SLOT_GUARD          0  /* Highest priority: The Black Hole */
#define PMP_SLOT_KERNEL_CODE    1  /* Read/Execute only */

#define PMP_READ (1)
#define PMP_WRITE (1 << 1)
#define PMP_EXECUTE (1 << 2)
#define PMP_LOCKED (1 << 7)

typedef enum {
    PMP_OFF   = (0 << 3),
    PMP_TOR   = (1 << 3),
    PMP_NA4   = (2 << 3),
    PMP_NAPOT = (3 << 3)
} pmp_mode_t;

typedef struct {
  pmp_mode_t mode;
  uint64_t pmpaddr0;
  uint64_t pmpaddr1;
} pmpinfo_t;

pmpinfo_t pmp_compute_encoding(pmp_mode_t mode, uint64_t start_addr, uint64_t end_addr);

void set_pmp_cfg(uint8_t index, uint8_t cfg_byte);

#define SET_PMP_ADDR(index, addr)\
    asm volatile("csrw pmpaddr" #index ", %0" : : "r"(addr));\

#define PMP_SET_NAPOT(reg_idx, pmpcfg_val, start, end) do { \
    uintptr_t _s = (uintptr_t)(start); \
    uintptr_t _e = (uintptr_t)(end); \
    pmpinfo_t _vals = pmp_compute_encoding(PMP_NAPOT, (_s), (_e)); \
    SET_PMP_ADDR(reg_idx, _vals.pmpaddr0); \
    set_pmp_cfg(reg_idx, (pmpcfg_val) | PMP_NAPOT); \
} while(0)

#define PMP_SET_TOR(reg_floor, reg_top, pmpcfg_val, start, end) do { \
    uintptr_t _s = (uintptr_t)(start); \
    uintptr_t _e = (uintptr_t)(end); \
    pmpinfo_t _vals = pmp_compute_encoding(PMP_TOR, (_s), (_e)); \
    SET_PMP_ADDR(reg_floor, _vals.pmpaddr0); \
    SET_PMP_ADDR(reg_top, _vals.pmpaddr1); \
    set_pmp_cfg(reg_top, (pmpcfg_val) | PMP_TOR); \
} while (0)

#endif
