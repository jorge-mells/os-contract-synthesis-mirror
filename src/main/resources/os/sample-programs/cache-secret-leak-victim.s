.section .text
.global _start

_start:
    la t0, _start
    addi t0, t0, 0x100
    
    # Counter: Run for 20,000 accesses, then exit
    li t2, 10000
loop:
    lw zero, 0(t0)
    addi t2, t2, -1
    bnez t2, loop
    
    # Exit syscall
    li a7, 93    # syscall: exit
    li a0, 0     # exit code 0
    ecall
