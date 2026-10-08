.section .text.boot
.global _start

_start:
    # start

    # end

    # syscall 93 (sys_exit)
    li a0, 0        # arg0 
    li a1, 0        # arg1
    li a2, 0        # arg2
    li a3, 0        # arg3
    li a4, 0        # arg4
    li a5, 0        # arg5
    li a6, 0
    li a7, 93
    ecall
