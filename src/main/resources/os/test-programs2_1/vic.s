.section .text.boot
.global _start

_start:
    # start
.word 0xadd6b317
.word 0x8d6eaba3
.word 0x871b8703
.word 0x091353e3
.word 0x00ba5113
.word 0x47e84393
.word 0x03b7b1b3
.word 0x00951d33
.word 0x2d7da3a3
.word 0x35b90ae7
.word 0xeea00513
.word 0x25200593
.word 0xd0b00613
.word 0xf7600693
.word 0x94900713
.word 0x10c00793
.word 0x9b800813
.word 0x05d00893
.word 0x00000073
.word 0xa4b00513
.word 0x61a00593
.word 0x36200613
.word 0xd4100693
.word 0x90000713
.word 0xb6500793
.word 0xaec00813
.word 0x05d00893
.word 0x00000073
.word 0x113661e3
.word 0x68539e23
.word 0x40d25233
.word 0x95942ba3
.word 0x02f922b3
.word 0x4113d533
.word 0x021bfd33
.word 0xb0900513
.word 0xa9e00593
.word 0xb5300613
.word 0x3be00693
.word 0x1e000713
.word 0x1ab00793
.word 0xc5000813
.word 0x07c00893
.word 0x00000073
.word 0xa1a00513
.word 0xf1800593
.word 0xa0800613
.word 0x28500693
.word 0x2c700713
.word 0x29200793
.word 0x6df00813
.word 0x05d00893
.word 0x00000073

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
