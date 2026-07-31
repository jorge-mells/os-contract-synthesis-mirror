.section .text.boot
.global _start

_start:
    # start

    # end

    # syscall 93 (sys_exit)
    li a0, 0
    li a7, 93
    ecall

