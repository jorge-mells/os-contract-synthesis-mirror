.section .data
msg_newline: .ascii "\n"
buffer:      .space 20      # Space to store the converted number string

.section .text
.globl _start

.equ SYS_WRITE, 64
.equ SYS_EXIT,  93

_start:
    li s0, 10       # n = 10 iterations
    li s1, 0        # F(0)
    li s2, 1        # F(1)

    # Print first number F(0)
    mv a0, s1
    jal ra, print_num

loop:
    # Print next number F(n)
    mv a0, s2
    jal ra, print_num

    # Calculate next Fibonacci
    add s3, s1, s2  # s3 = s1 + s2
    mv s1, s2       # s1 = s2
    mv s2, s3       # s2 = s3

    addi s0, s0, -1 # Decrement counter
    bnez s0, loop   # Loop until s0 == 0

exit:
    li a7, SYS_EXIT
    li a0, 0
    ecall

# --- Subroutine: print_num ---
# Input: a0 = integer to print
print_num:
    addi sp, sp, -32    # Save stack
    sd ra, 24(sp)
    
    la t0, buffer
    addi t0, t0, 19     # Start at the end of the buffer (backward)
    li t1, 10           # Divisor
    li t3, 0            # Character counter

convert_loop:
    rem t2, a0, t1      # Get remainder (digit)
    addi t2, t2, 48     # Convert to ASCII ('0' = 48)
    sb t2, 0(t0)        # Store character
    addi t0, t0, -1     # Move pointer back
    addi t3, t3, 1      # Increment length
    div a0, a0, t1      # Divide number by 10
    bnez a0, convert_loop

    # Call sys_write for the number
    li a7, SYS_WRITE
    li a0, 1            # stdout
    addi a1, t0, 1      # Buffer starts at last stored char
    mv a2, t3           # Length
    ecall

    # Print a newline
    li a7, SYS_WRITE
    li a0, 1
    la a1, msg_newline
    li a2, 1
    ecall

    ld ra, 24(sp)       # Restore stack
    addi sp, sp, 32
    ret