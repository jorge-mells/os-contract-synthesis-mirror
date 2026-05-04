.section .text
.globl _start

_start:
    # We are currently at 0x82000000 (example)
    # Let's try to jump to 0x82001008 (4096 + 8 bytes away)
    # simple arbitrary jump

    li t0, 0x85001008
    jr t0              # This should trigger an Instruction Access Fault
                       # if your PMP range ends at 0x82001000.
