#ifndef SCHEDULER_H
#define SCHEDULER_H

#define INTERVAL 1000000 // cpu dependent
#define MAX_PROCS 10
#define PROG_SIZE  4096
#define STACK_SIZE 4096
#define GUARD_SIZE 1024
#define BLOCK_SIZE (PROG_SIZE + STACK_SIZE + GUARD_SIZE)


#ifndef __ASSEMBLER__
#include <stdint.h>
#include <stdbool.h>
typedef struct pcb {
    /* OFFSET 0: Hardware Timer Data
       Guaranteed by C standard to be at the start of the struct. */
    uint64_t time_slice;

    uint64_t context[32];

    uint64_t pid;
    uint64_t state;
    uint64_t pc;
    // NOTE: there's no #define for the offset of the following members of the struct
    uint64_t base_addr;
    
} pcb_t;
#define PCB_STATE_UNUSED   0
#define PCB_STATE_USED     1
#define PCB_STATE_SLEEPING 2
#define PCB_STATE_READY 3
#define PCB_STATE_RUNNING  4
#define PCB_STATE_ZOMBIE   5

extern uint64_t kernel_sp;
extern pcb_t pcb_table[MAX_PROCS];
extern pcb_t *current_running_pcb;

void load_programs(uint64_t start_addr, uint64_t space_allocated, int num_requested);
void scheduler();
#endif

/* PCB Header */
#define PCB_TIME_SLICE    0

/* Context Array (Registers x0 through x31) 
   Offset = Header (8) + (RegIndex * 8) */
#define PCB_CONTEXT_BASE  8
#define PCB_REG_X0        8
#define PCB_REG_RA        16   /* x1 */
#define PCB_REG_SP        24   /* x2 */
#define PCB_REG_GP        32   /* x3 */
#define PCB_REG_TP        40   /* x4 */
#define PCB_REG_T0        48   /* x5 */
#define PCB_REG_T1        56   /* x6 */
#define PCB_REG_T2        64   /* x7 */
#define PCB_REG_S0        72   /* x8 / fp */
#define PCB_REG_S1        80   /* x9 */
#define PCB_REG_A0        88   /* x10 */
#define PCB_REG_A1        96   /* x11 */
#define PCB_REG_A2        104  /* x12 */
#define PCB_REG_A3        112  /* x13 */
#define PCB_REG_A4        120  /* x14 */
#define PCB_REG_A5        128  /* x15 */
#define PCB_REG_A6        136  /* x16 */
#define PCB_REG_A7        144  /* x17 */
#define PCB_REG_S2        152  /* x18 */
#define PCB_REG_S3        160  /* x19 */
#define PCB_REG_S4        168  /* x20 */
#define PCB_REG_S5        176  /* x21 */
#define PCB_REG_S6        184  /* x22 */
#define PCB_REG_S7        192  /* x23 */
#define PCB_REG_S8        200  /* x24 */
#define PCB_REG_S9        208  /* x25 */
#define PCB_REG_S10       216  /* x26 */
#define PCB_REG_S11       224  /* x27 */
#define PCB_REG_T3        232  /* x28 */
#define PCB_REG_T4        240  /* x29 */
#define PCB_REG_T5        248  /* x30 */
#define PCB_REG_T6        256  /* x31 */

/* Trailing Metadata 
   Offset = Header (8) + Context (32 * 8 = 256) + Previous Fields */
#define PCB_PID           264
#define PCB_STATE         272
#define PCB_PC            280

#endif