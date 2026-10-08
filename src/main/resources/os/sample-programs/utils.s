# useful assembly routines you might use
# input in a0
print_int:
    li t0, 10
    mv t1, a0
    li t3, 0

    bnez t1, loop
    li a7, 64
    li a0, 1
    li a2, 1
    la a1, msg_zero
    ecall
    j exit_print

loop:
    beq t1, zero, print_loop
    rem t2, t1, t0
    addi t2, t2, 48
    addi sp, sp, -1
    sb t2, 0(sp)
    div t1, t1, t0
    addi t3, t3, 1
    j loop

print_loop:
    beq t3, zero, exit_print
    mv a1, sp
    li a7, 64
    li a0, 1
    li a2, 1
    ecall
    addi sp, sp, 1
    addi t3, t3, -1
    j print_loop

exit_print:
    li a7, 64        # Syscall for write
    li a0, 1         # File descriptor 1 (stdout)
    la a1, newline   # Load address of the newline string
    li a2, 1         # Length of string (1 byte for \n)
    ecall
    ret
