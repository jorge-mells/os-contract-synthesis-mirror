.section .tohost, "aw", @progbits
.align 3
.global tohost
tohost: .dword 0

.global fromhost
fromhost: .dword 0