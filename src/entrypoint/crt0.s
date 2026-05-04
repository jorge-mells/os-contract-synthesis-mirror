.section .init, "ax"
.global _start
_start:
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
  sd zero, 0(t0)       # Store 64-bit zero
  addi t0, t0, 8       # Move to next doubleword
  j bss_zero_loop

bss_done:
  jal zero, kmain
  .cfi_endproc
  .end
