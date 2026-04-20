#include "interrupts.h"
#include "scheduler.h"

.equ CLINT_MTIMECMP, 0x02004000 # cpu dependent
.equ CLINT_MTIME,    0x0200bff8 # cpu dependent

.section .rodata
  log_exception_msg: .asciz "Exception %u at %p. Skipping...\n"
  log_interrupt_msg: .asciz "Interrupt %u handled.\n"
  log_debug: .asciz "here\n"

.section .text

.global setup_traps
setup_traps:
  # Check if MTIE (bit 7) is being enabled in a1
  li t0, 0x80 # Load mask for bit 7 (MTIE)
  and t1, a1, t0 
  beqz t1, .skip_timer 

  # Setup mtimecmp
  li t2, CLINT_MTIMECMP
  li t3, CLINT_MTIME
  ld t4, 0(t3)             # Load current mtime
  li t5, INTERVAL
  add t4, t4, t5           # t4 = mtime + INTERVAL
  sd t4, 0(t2)             # mtimecmp = t4

.skip_timer:
  csrs mie, a1
  csrs mstatus, a0
  csrw mtvec, a2
  ret
  

.global main_trap_handler
.align 4
main_trap_handler:
  # prologue
  addi sp, sp, -32 # NOTE: instead of using user_stack, switch later to kernel_stack
  sd ra, 0(sp)
  sd a0, 8(sp)
  sd a1, 16(sp)
  sd a2, 24(sp)

  csrr a1, mcause
  // jump to right handler if interrupt or exception
  blt a1, zero, .main_interrupt_handler

.main_exception_handler:
  # print a log message for now
  # NOTE: it's a lot more complex though since not only ecalls and ebreaks trigger exceptions but for now assume
  # only ebreaks and ecalls trigger exceptions
  la a0, log_exception_msg
  csrr a2, mepc
  call kprintf
  # do the actual stuff
  # addi a2, a2, 2
  # csrw mepc, a2

  # WARN: temporary
  call poweroff

  j .end_main_trap_handler


.main_interrupt_handler:
  # # print a log message
  # la a0, log_interrupt_msg
  # call kprintf
  # Clear the MSB to get the pure ID
  li s0, 0x7fffffffffffffff
  and a1, a1, s0

  # jump to right interrupt handler
  li s0, 3 # Code 3 = Machine Software Interrupt
  beq a1, s0, .software_interrupt_handler

  li s0, 7 # Code 7 = Machine Timer Interrupt
  beq a1, s0, .timer_interrupt_handler

  j .other_interrupts_handler      # Fallback for anything else

.software_interrupt_handler:
  j .end_main_trap_handler

.extern current_running_pcb
.timer_interrupt_handler:
  # get back all regs into their position(basically epilogue)
  # save regs into correct pcb location; you can probably use #define for this

  call scheduler

  # reset mtimecmp
  # setup timer addresses
  li s2, CLINT_MTIMECMP
  li s3, CLINT_MTIME
  # Get the time slice from the NEW current_running_pcb
  la s5, current_running_pcb
  ld s5, 0(s5)
  ld s5, 0(s5)
  # Calculate and set the new alarm
  ld s4, 0(s3)             # s4 = current mtime
  add s4, s4, s5           # s4 = mtime + time_slice
  sd s4, 0(s2)             # mtimecmp = s4
  
  la a0, log_debug
  call kprintf
  j .end_main_trap_handler

.other_interrupts_handler:
  j .end_main_trap_handler

.end_main_trap_handler:
  # epilogue
  ld ra, 0(sp)
  ld a0, 8(sp)
  ld a1, 16(sp)
  ld a2, 24(sp)
  addi sp, sp, 32
  mret