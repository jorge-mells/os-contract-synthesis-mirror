.text
.global _start

_start:
    li x1, 1000    # Load loop counter (1 million iterations)
    li x2, 2          # Operand 1
    li x3, 3          # Operand 2
    li x4, 0          # Result accumulator

loop:
    add x4, x2, x3    # Perform multiplication: x4 = x2 * x3
    addi x1, x1, -1   # Decrement counter
    bnez x1, loop     # If counter != 0, jump back to loop

    # Exit (simulated environment exit)
    li x10, 0
    li x17, 93        # sys_exit
    ecall
