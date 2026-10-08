# assumption 1: attacker starts in two proc system

.section .text
.global _start

_start:
    la s0, _start
    addi s0, s0, 0x100
    
    
    # Counter: Perform 1,000 probes
    li s2, 10 

prime_and_probe:
    # li t3, 0x430000  # mtime address ibex
    li t3, 0x0200bff8 # mtime address cva6 & qemu
    # 1. PRIME (Using 4 ways due to 4KB limit)
    li t1, 1024          # Stride
    
    lw zero, 0(s0)       # Way 0
    add s0, s0, t1       # s0 = s0 + 1024
    
    lw zero, 0(s0)       # Way 1
    add s0, s0, t1       # s0 = s0 + 1024
    
    lw zero, 0(s0)       # Way 2
    add s0, s0, t1       # s0 = s0 + 1024
    
    lw zero, 0(s0)       # Way 3
    
    # Optional: Reset s0 to original base if needed
    sub s0, s0, t1       # s0 = s0 - 1024
    sub s0, s0, t1
    sub s0, s0, t1

    # 2. YIELD
    li a7, 124
    ecall

    # 3. PROBE
    lw t0, 0(t3)
    # rdcycle t0
    lw zero, 0(s0)
    lw t1, 0(t3)
    # rdcycle t1
    
    sub s1, t1, t0
    mv a0, s1
    call print_int
    
    # (Optional: Store s1 in a buffer or output via syscall)
    
    addi s2, s2, -1
    bnez s2, prime_and_probe

exit:
    # Exit cleanly
    li a7, 93
    li a0, 0
    ecall

.include "utils.s" # don't move to top!!

.data
msg_zero: .ascii "0"
newline: .string "\n"
