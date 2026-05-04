#include "traps.h"
#include "../platform/platform.h"
#include "../process-mgmt/scheduler.h"
#include "../syscalls/handler.h"
#include "../syscalls/syscalls.h"

.section .rodata
  log_exception_msg: .asciz "Exception %u at %p.\n"
  log_interrupt_msg: .asciz "Interrupt %u handled.\n"
  log_interrupt_nohandle_msg: .asciz "Interrupt %u triggered. Exiting...\n"
  log_debug: .asciz "here\n"
  log_print: .asciz "val: %u\n"

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
.align 4
# NOTE: unimplemented interrupt/exception blocks just exit(poweroff) the kernel
main_trap_handler:
  # prologue
  # unfortunately, since we can't tell what regs were being used before the trap(there's no interrupt/exception abi, all have
  # to be saved

  # store sp for user before doing anything else
  csrw mscratch, sp 
  la sp, current_running_pcb
  ld sp, 0(sp)
  addi sp, sp, PCB_CONTEXT_BASE # get address of context base

  # store pcb regs
  sd x0, 0(sp)
  sd x1, 8(sp)
  sd x2, 16(sp)
  # sd x3, 24(sp)
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

  # store user sp in pcb
  mv t0, sp
  csrr sp, mscratch # get user sp back
  sd x3, 24(t0)

  # save user pc; so that syscalls and other code can access and modify this as required
  la t0, current_running_pcb   # Get address of the pointer
  ld t0, 0(t0)                 # t0 now holds the actual base address of the PCB
  # Calculate the specific address within the PCB for the PC
  addi t1, t0, PCB_PC          # t1 = address of pcb->pc
  # Save the exception program counter (user's PC) into the PCB
  csrr t0, mepc          # CSR Read: t0 = mepc
  sd t0, 0(t1)           # Store t0 into the address held in t1 (pcb->pc)

  # load kernel stack pointer and gp
  la t0, kernel_sp
  ld sp, 0(t0)
  la t0, kernel_gp
  ld gp, 0(t0)

  # jump to right handler if interrupt or exception
  csrr a1, mcause
  blt a1, zero, .main_interrupt_handler

.main_exception_handle:
  # print a log message
  la a0, log_exception_msg
  # clear the msb to get the pure id
  csrr a1, mcause
  li s0, 0x7fffffffffffffff
  and a1, a1, s0
  csrr a2, mepc
  call kprintf

  # kprintf messes up aX regs hence I reload
  csrr a1, mcause
  li s0, 0x7fffffffffffffff
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
  li s0, 0x7fffffffffffffff
  and a1, a1, s0
  call kprintf

  # kprintf messes up aX regs
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
  ld s5, 0(s5)
  ld s5, 0(s5)
  # calculate and set the new alarm
  ld s4, 0(s3) # s4 = current mtime
  add s4, s4, s5 # s4 = mtime + time_slice
  sd s4, 0(s2) # mtimecmp = s4
  
  j .end_main_trap_handler

.other_interrupts_handler:
  call cleanup_and_exit

.end_main_trap_handler:
  # load pc
  # 1. Get the base address of the current PCB
  la t0, current_running_pcb
  ld t0, 0(t0)                 # t0 = pointer to current PCB
  # 2. Load the saved PC value from the PCB into a scratch register
  # Using the same PCB_PC offset used during the save
  ld t1, PCB_PC(t0)            # t1 = pcb->pc
  # 3. Write the value into the Machine Exception Program Counter
  csrw mepc, t1

  # save kernel stack pointer and global pointer
  la t0, kernel_sp
  sd sp, 0(t0)
  la t0, kernel_gp
  sd gp, 0(t0)

  # get location of regs stored
  la sp, current_running_pcb
  ld sp, 0(sp)
  addi sp, sp, PCB_CONTEXT_BASE

  # epilogue
  ld x0, 0(sp)
  ld x1, 8(sp)
  ld x2, 16(sp)
  # ld x3, 24(sp)
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
  # addi sp, sp, 256

  # load user sp back
  ld x3, 24(sp)
  mret
