.global _start
.extern strlen
.extern strcmp

.section .text

_start:
    movi r15, 0x0000F000

    movi r0, word_a
    call strlen
    mov  r8, r1

    movi r0, word_a
    movi r1, word_b
    call strcmp
    mov  r9, r2

    halt

.section .data

word_a:
    .byte 'H', 'e', 'l', 'l', 'o', 0
word_b:
    .byte 'H', 'e', 'l', 'l', 'o', 0
