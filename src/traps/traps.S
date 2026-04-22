#include "traps.h"
#include "../process-mgmt/scheduler.h"

# .equ INTERVAL, 10000000 # cpu dependent
.equ CLINT_MTIMECMP, 0x02004000 # cpu dependent
.equ CLINT_MTIME,    0x0200bff8 # cpu dependent

.section .rodata
  log_exception_msg: .asciz "Exception %u at %p. Exiting...\n"
  log_interrupt_msg: .asciz "Interrupt %u handled.\n"
  log_interrupt_nohandle_msg: .asciz "Interrupt %u triggered. Exiting...\n"
  log_debug: .asciz "here\n"

.section .text

// see the signature of setup_traps to know what is in a0-a3 according to risc-v abi
.global setup_traps
setup_traps:
  # Check if mtie is being enabled in a1
  li t0, 0x80 # load mask for mtie
  and t1, a1, t0 
  beqz t1, .skip_timer 

  # Setup mtimecmp
  li t2, CLINT_MTIMECMP
  li t3, CLINT_MTIME
  ld t4, 0(t3) # t4 = &mtime
  li t5, INTERVAL # t5 = mtime
  add t4, t4, t5 # add mtime and interval to get mtimecmp
  sd t4, 0(t2) # mtimecmp = t4

.skip_timer:
  csrw mtvec, a2
  csrw mepc, a3
  csrs mie, a1
  csrs mstatus, a0
  beqz a3, .do_ret
  # NOTE: don't forget to later change stack pointer for user progs
  mret

.do_ret:
  ret
  

.global main_trap_handler
.align 4
# NOTE: unimplemented interrupt/exception blocks just exit
main_trap_handler:
  # prologue
  # unfortunately, since we can't tell what regs were being used before the trap(there's no interrupt/exception abi, all have
  # to be saved
  # put this data in some protected location instead. we could move it directly into pcb block and then out of pcb block.
  addi sp, sp, -264 # NOTE: instead of using user_stack, switch later to kernel_stack
  sd x0, 0(sp)
  sd x1, 8(sp)
  sd x2, 16(sp)
  sd x3, 24(sp)
  sd x4, 32(sp)
  sd x5, 40(sp)
  sd x6, 48(sp)
  sd x7, 56(sp)
  sd x8, 64(sp)
  sd x9, 72(sp)
  sd x10, 80(sp)
  sd x11, 88(sp)
  sd x12, 96(sp)
  sd x13, 104(sp)
  sd x14, 112(sp)
  sd x15, 120(sp)
  sd x16, 128(sp)
  sd x17, 136(sp)
  sd x18, 144(sp)
  sd x19, 152(sp)
  sd x20, 160(sp)
  sd x21, 168(sp)
  sd x22, 176(sp)
  sd x23, 184(sp)
  sd x24, 192(sp)
  sd x25, 200(sp)
  sd x26, 208(sp)
  sd x27, 216(sp)
  sd x28, 224(sp)
  sd x29, 232(sp)
  sd x30, 240(sp)
  sd x31, 248(sp)

  csrr a1, mcause
  # jump to right handler if interrupt or exception
  blt a1, zero, .main_interrupt_handler

.main_exception_handle:
  # print a log message
  la a0, log_exception_msg
  csrr a2, mepc
  call kprintf
  # j .end_main_trap_handler
  call cleanup_and_exit


.main_interrupt_handler:
  # print a log message
  la a0, log_interrupt_msg
  call kprintf
  # clear the msb to get the pure id
  csrr a1, mcause
  li s0, 0x7fffffffffffffff
  and a1, a1, s0

  # jump to right interrupt handler
  li s0, 3
  beq a1, s0, .software_interrupt_handler

  li s0, 7
  beq a1, s0, .timer_interrupt_handler

  j .other_interrupts_handler # fallback for anything else

.software_interrupt_handler:
  call cleanup_and_exit
  # j .end_main_trap_handler

.extern current_running_pcb
.timer_interrupt_handler:
  # get back all regs into their position(basically epilogue)
  # save regs into correct pcb location

  call scheduler # scheduler changes current_running_pcb

  # restore the regs.(basically prologue)

  # reset mtimecmp
  # WARN: the edge case of mtime being too large and overflowing is not handled
  # setup timer addresses
  li s2, CLINT_MTIMECMP
  li s3, CLINT_MTIME
  # get the time slice from the NEW current_running_pcb
  la s5, current_running_pcb
  ld s5, 0(s5)
  ld s5, 0(s5)
  # calculate and set the new alarm
  ld s4, 0(s3) # s4 = current mtime
  add s4, s4, s5 # s4 = mtime + time_slice
  sd s4, 0(s2) # mtimecmp = s4
  
  j .end_main_trap_handler

.other_interrupts_handler:
  call cleanup_and_exit
  # j .end_main_trap_handler

.end_main_trap_handler:
  # epilogue
  ld x0, 0(sp)
  ld x1, 8(sp)
  ld x2, 16(sp)
  ld x3, 24(sp)
  ld x4, 32(sp)
  ld x5, 40(sp)
  ld x6, 48(sp)
  ld x7, 56(sp)
  ld x8, 64(sp)
  ld x9, 72(sp)
  ld x10, 80(sp)
  ld x11, 88(sp)
  ld x12, 96(sp)
  ld x13, 104(sp)
  ld x14, 112(sp)
  ld x15, 120(sp)
  ld x16, 128(sp)
  ld x17, 136(sp)
  ld x18, 144(sp)
  ld x19, 152(sp)
  ld x20, 160(sp)
  ld x21, 168(sp)
  ld x22, 176(sp)
  ld x23, 184(sp)
  ld x24, 192(sp)
  ld x25, 200(sp)
  ld x26, 208(sp)
  ld x27, 216(sp)
  ld x28, 224(sp)
  ld x29, 232(sp)
  ld x30, 240(sp)
  ld x31, 248(sp)
  addi sp, sp, 256
  mret

.global setup_memory_protection
setup_memory_protection:
  # 1. Define the Range (The "Whole World")
  li t0, -1              # All ones: 0xFFFFFFFFFFFFFFFF
  csrw pmpaddr0, t0

  # 2. Define Permissions (NAPOT, R, W, X)
  li t1, 0x1F            # 0x1F = 0001 1111 (NAPOT + RWX)
  csrw pmpcfg0, t1

  ret

.global setup_memory_protection_c
setup_memory_protection_c: