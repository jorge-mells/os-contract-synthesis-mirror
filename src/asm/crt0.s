.section .init, "ax"
.global _start
_start:
#   csrr t0, misa
#   srli t0, t0, 30
#   li t1, 1
#   beq t0, t1, is_rv32

# exit_not_rv32:
#   li t0, 1
#   la t1, tohost
#   sw t0, 0(t1)

# is_rv32:
  .cfi_startproc
  .cfi_undefined ra
  .option push
  .option norelax
  la gp, __global_pointer$
  .option pop
  la sp, __stack_top
  add s0, sp, zero

  # Zero out BSS
  la t0, __bss_start    # Start address
  la t1, __BSS_END__      # End address

bss_zero_loop:
  beq t0, t1, bss_done # If start == end, we are finished
  sw zero, 0(t0)       # Store XLEN-bit zero
  addi t0, t0, 4       # Move to next word
  j bss_zero_loop

bss_done:
  jal zero, kmain
  .cfi_endproc
  .end
