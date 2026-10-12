# Assume t0 holds the secret bit (0 or 1)
# Assume a7 = 124 (syscall: sched_yield)

li t0, 0
beq t0, zero, exit   # If secret bit is 0, skip the "busy" loop

# Busy loop (Simulates a secret-dependent operation)
li t1, 2000            # Large loop counter
loop:
    addi t1, t1, -1
    bnez t1, loop

exit:
    li a0, 0            # status 0
    li a7, 93
    ecall
