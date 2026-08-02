.global _start
.extern strlen
.section .text
_start:
    movi r15, 0x0000F000
    movi r0, message
    call strlen
    halt
.section .data
message:
    .byte 'H', 'e', 'l', 'l', 'o', 0
