#include "traps.h"
#include "../common/common.h"
#include "../platform/platform.h"
#include "../process-mgmt/scheduler.h"
#include "../syscalls/handler.h"
#include "../syscalls/syscalls.h"

// NOTE: use klog instead of call klog, to save up on instructions
#if LOG_ENABLED
    .macro klog_call
        call klog_impl
    .endm
#else
    .macro klog_call
        /* Do nothing */
    .endm
#endif

// WARN: DON'T TOUCH THIS. SOME ONE WILL PROBABLY DO SO I WON'T SAY WHY. GOOD LUCK DEBUGGING.
// JUST KIDDING: stupid compiler optimisations as usual. it'll end up using global pointer if you do,
// which isn't quite correct in here since you're coming from another user program.
.option norelax 

.section .rodata
  log_exception_msg: .asciz "\nException %u at %p.\n"
  log_interrupt_msg: .asciz "\nInterrupt %u handled.\n"
  log_interrupt_nohandle_msg: .asciz "\nInterrupt %u triggered. Exiting...\n"
  log_debug: .asciz "here\n"
  log_print: .asciz "val: %p\n"

.section .text

// NOTE: I am currently not paying full attention to which regs are being used, hence there might be bugs there.
// see the signature of the various functions defined here in traps.h
.extern current_running_pcb
.extern kernel_sp
.extern kernel_gp
.global main_trap_handler





##############################
###### Function call #########
##############################
.align 8
# NOTE: unimplemented interrupt/exception blocks just exit(poweroff) the kernel
main_trap_handler:
# fake 'slide' for cpus with only vectored mode
  j trap_entry_common    # Offset 0: Exception / Reset
  .align 2
  j trap_entry_common    # Offset 4: Software Interrupt
  .align 2
  j trap_entry_common    # Offset 8: Reserved
  .align 2
  j trap_entry_common    # Offset 12: Machine Software Interrupt
  .align 2
  j trap_entry_common    # Offset 16: Reserved
  .align 2
  j trap_entry_common    # Offset 20: Supervisor Timer Interrupt
  .align 2
  j trap_entry_common    # Offset 24: Reserved
  .align 2
  j trap_entry_common    # Offset 28: Machine Timer Interrupt (The 2202 culprit)

trap_entry_common:
  # prologue
  # unfortunately, since we can't tell what regs were being used before the trap(there's no interrupt/exception abi, all have
  # to be saved

  # store sp for user before doing anything else
  csrw mscratch, sp 
  la sp, current_running_pcb
  LREG sp, 0(sp)
  addi sp, sp, PCB_CONTEXT_BASE # get address of context base

  # store pcb regs
  SREG x0, 0(sp)
  SREG x1, 8(sp)
  # SREG x2, 16(sp)
  SREG x3, 24(sp)
  SREG x4, 32(sp)
  SREG x5, 40(sp)
  SREG x6, 48(sp)
  SREG x7, 56(sp)
  SREG x8, 64(sp)
  SREG x9, 72(sp)
  SREG x10, 80(sp)
  SREG x11, 88(sp)
  SREG x12, 96(sp)
  SREG x13, 104(sp)
  SREG x14, 112(sp)
  SREG x15, 120(sp)
  SREG x16, 128(sp)
  SREG x17, 136(sp)
  SREG x18, 144(sp)
  SREG x19, 152(sp)
  SREG x20, 160(sp)
  SREG x21, 168(sp)
  SREG x22, 176(sp)
  SREG x23, 184(sp)
  SREG x24, 192(sp)
  SREG x25, 200(sp)
  SREG x26, 208(sp)
  SREG x27, 216(sp)
  SREG x28, 224(sp)
  SREG x29, 232(sp)
  SREG x30, 240(sp)
  SREG x31, 248(sp)

  # store user sp in pcb
  mv t0, sp
  csrr sp, mscratch # get user sp back
  SREG x2, 16(t0)

  # save user pc; so that syscalls and other code can access and modify this as required
  la t0, current_running_pcb   # Get address of the pointer
  LREG t0, 0(t0)                 # t0 now holds the actual base address of the PCB
  # Calculate the specific address within the PCB for the PC
  addi t1, t0, PCB_PC          # t1 = address of pcb->pc
  # Save the exception program counter (user's PC) into the PCB
  csrr t0, mepc          # CSR Read: t0 = mepc
  SREG t0, 0(t1)           # Store t0 into the address held in t1 (pcb->pc)

  # load kernel stack pointer and gp
  la t0, kernel_sp
  LREG sp, 0(t0)
  la t0, kernel_gp
  LREG gp, 0(t0)

  # jump to right handler if interrupt or exception
  csrr a1, mcause
  blt a1, zero, .main_interrupt_handler

.main_exception_handle:
  # print a log message
  la a0, log_exception_msg
  # clear the msb to get the pure id
  csrr a1, mcause
  # li s0, 0x7fffffffffffffff
  li s0, -1 # better version of the above to support 32/64 bit systems
  srli s0, s0, 1
  and a1, a1, s0
  csrr a2, mepc
  klog_call

  # klog messes up aX regs hence I reload
  csrr a1, mcause
  # li s0, 0x7fffffffffffffff
  li s0, -1 # better version of the above to support 32/64 bit systems
  srli s0, s0, 1
  and a1, a1, s0

  # jump to right exception handler
  li s0, 8
  beq a1, s0, .ecall_exception_handler

  j .other_exceptions_handler

.ecall_exception_handler:
  call handle_syscall
  j .end_main_trap_handler

.other_exceptions_handler:
  li a0, 1
  call sys_exit
  j .end_main_trap_handler


.main_interrupt_handler:
  # print a log message
  la a0, log_interrupt_msg
  # clear the msb to get the pure id
  csrr a1, mcause
  # li s0, 0x7fffffffffffffff
  li s0, -1 # better version of the above to support 32/64 bit systems
  srli s0, s0, 1
  and a1, a1, s0
  klog_call

  # klog messes up aX regs
  csrr a1, mcause
  # li s0, 0x7fffffffffffffff
  li s0, -1 # better version of the above to support 32/64 bit systems
  srli s0, s0, 1
  and a1, a1, s0

  # jump to right interrupt handler
  li s0, 3
  beq a1, s0, .software_interrupt_handler

  li s0, 7
  beq a1, s0, .timer_interrupt_handler

  j .other_interrupts_handler # fallback for anything else

.software_interrupt_handler:
  call cleanup_and_exit

.timer_interrupt_handler:
  call scheduler # scheduler changes current_running_pcb

  # reset mtimecmp
  # WARN: the edge case of mtime being too large and overflowing is not handled
  # setup timer addresses
  li s2, CLINT_MTIMECMP
  li s3, CLINT_MTIME
  # get the time slice from the NEW current_running_pcb
  la s5, current_running_pcb
  add s5, s5, PCB_TIME_SLICE
  LREG s5, 0(s5)
  LREG s5, 0(s5)
  # calculate and set the new alarm
  LREG s4, 0(s3) # s4 = current mtime
  add s4, s4, s5 # s4 = mtime + time_slice
  SREG s4, 0(s2) # mtimecmp = s4
  
  j .end_main_trap_handler

.other_interrupts_handler:
  call cleanup_and_exit

.end_main_trap_handler:
  # load pc
  # 1. Get the base address of the current PCB
  la t0, current_running_pcb
  LREG t0, 0(t0)                 # t0 = pointer to current PCB
  # 2. Load the saved PC value from the PCB into a scratch register
  # Using the same PCB_PC offset used during the save
  LREG t1, PCB_PC(t0)            # t1 = pcb->pc
  # 3. Write the value into the Machine Exception Program Counter
  csrw mepc, t1

  # save kernel stack pointer and global pointer
  la t0, kernel_sp
  SREG sp, 0(t0)
  la t0, kernel_gp
  SREG gp, 0(t0)

  # get location of regs stored
  la sp, current_running_pcb
  LREG sp, 0(sp)
  addi sp, sp, PCB_CONTEXT_BASE

  # epilogue
  LREG x0, 0(sp)
  LREG x1, 8(sp)
  # ld x2, 16(sp)
  LREG x3, 24(sp)
  LREG x4, 32(sp)
  LREG x5, 40(sp)
  LREG x6, 48(sp)
  LREG x7, 56(sp)
  LREG x8, 64(sp)
  LREG x9, 72(sp)
  LREG x10, 80(sp)
  LREG x11, 88(sp)
  LREG x12, 96(sp)
  LREG x13, 104(sp)
  LREG x14, 112(sp)
  LREG x15, 120(sp)
  LREG x16, 128(sp)
  LREG x17, 136(sp)
  LREG x18, 144(sp)
  LREG x19, 152(sp)
  LREG x20, 160(sp)
  LREG x21, 168(sp)
  LREG x22, 176(sp)
  LREG x23, 184(sp)
  LREG x24, 192(sp)
  LREG x25, 200(sp)
  LREG x26, 208(sp)
  LREG x27, 216(sp)
  LREG x28, 224(sp)
  LREG x29, 232(sp)
  LREG x30, 240(sp)
  LREG x31, 248(sp)
  # addi sp, sp, 256

  # load user sp back
  LREG x2, 16(sp)
  mret
