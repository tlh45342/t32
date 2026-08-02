.global _start
.extern string_to_hex
.section .text
_start:
    movi r15, 0x0000F000
    movi r8, 0x88888888
    movi r9, 0x99999999
    movi r10, 0x10101010
    movi r11, 0x11111111
    movi r12, 0x12121212
    movi r13, 0x13131313
    movi r14, 0x14141414
    movi r0, input
    call string_to_hex
    halt
.section .data
input:
    .byte '1', '2', '3', '4', 'A', 'B', 'C', 'D', 0
