; libt32 putchar -- T32 ABI 0.1
; Arguments: r0=character byte
; Returns:   r0=character byte
; Preserves: r8-r15
; Clobbers:  r1-r7
;
; Text console is 80 columns of one-byte cells. Newline advances the
; library-owned cursor to column 0 of the next row.
.section .text
.global putchar

putchar:
    mov  r6, r0
    movi r4, t32_putchar_cursor
    ldw  r5, [r4]

    movi r1, 10
    xor  r1, r6, r1
    jz   r1, putchar_newline

    stb  r6, [r5]
    addi r5, r5, 1
    stw  r5, [r4]
    mov  r0, r6
    ret

putchar_newline:
    movi r1, 0x90000000
    sub  r2, r5, r1
    movi r3, 80
    div  r2, r2, r3
    addi r2, r2, 1
    mul  r2, r2, r3
    add  r5, r1, r2
    stw  r5, [r4]
    mov  r0, r6
    ret

.section .data
t32_putchar_cursor:
    .word 0x90000500
