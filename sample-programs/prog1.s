.section .text.boot
.global _start

_start:

    # PC-relative load of the message address
    # Result: a1 = current_pc + offset_to_msg
    la a1, msg          
    
    # syscall 64 (sys_write)
    # a0 = 1 (stdout), a1 = buffer, a2 = length
    li a0, 1
    # We use a simple loop in the kernel for \0, so length in a2 
    # can be 0 or ignored if your kernel doesn't use it.
    la t0, msg
    la a2, msg_end
    sub a2, a2, t0    # a2 = address_of_end - address_of_start (length)
    li a7, 64
    ecall

    # syscall 93 (sys_exit)
    li a0, 0            # status 0
    li a7, 93
    ecall

.section .rodata
msg:
    .asciz "Hello 1\n"
msg_end:
    