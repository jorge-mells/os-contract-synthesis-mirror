.section .text.boot
.global _start

_start:
    # start
.word 0x9e035f97
.word 0x0219f3b3
.word 0x917083e7
.word 0xcb013693
.word 0x6ae21697
.word 0x83eb94a3
.word 0x41bfd993
.word 0x0008b393
.word 0x40d00e33
.word 0xd5c00513
.word 0x75200593
.word 0xdf000613
.word 0x85c00693
.word 0xeb200713
.word 0x40900793
.word 0xe0100813
.word 0x07c00893
.word 0x00000073
.word 0x0e5d4297
.word 0x68762a13
.word 0xa36ca803
.word 0x6378ae03
.word 0x01114c33
.word 0x03b582b3
.word 0x0082ab33
.word 0x418c5333
.word 0x9c93ef13
.word 0x03cedcb3

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
