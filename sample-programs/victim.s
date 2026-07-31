.section .text
.globl _start

_start:
    # --- Training Phase ---
    li t0, 1            # t0 = 1
    li t1, 1            # t1 = 1
    li t2, 500          # Loop counter
train_loop:
    beq t0, t1, target  # Predicts taken
    addi t2, t2, -1
    bnez t2, train_loop

    # --- Victim Phase ---
    li t0, 1
    li t1, 1            # Condition will fail (1 != 0)
    
    # This will be mispredicted as "Taken"
    beq t0, t1, target  

    # --- Correct Path (Fall-Through) ---
    addi t3, t3, 1      # This is the path we want
    j exit

target:
    # --- Wrong Path (Speculative) ---
    addi t4, t4, 1
    mul  t5, t4, t6
    lw   s0, -4(sp)      # Potential memory access
    j exit              # Return to flow

exit:
    li a7, 93           # syscall: exit
    li a0, 0            # status: 0
    ecall
