; libt32 putchar -- T32 ABI 0.1
; Arguments: r0=character byte
; Returns:   r0=character byte
; Preserves: r8-r15
; Clobbers:  r1-r7
;
; Text console:
;   base        0x90000000
;   columns     80
;   rows        25
;   end         0x900007D0 (one byte past framebuffer)
;
; The library-owned cursor is always kept inside the framebuffer.
; LF advances to column 0 of the next row.
; CR returns to column 0 of the current row.
; Ordinary output wraps naturally at 80 columns.
; Advancing beyond the bottom row scrolls rows 1..24 upward and
; clears row 24 with spaces.
.section .text
.global putchar

putchar:
    mov  r6, r0
    movi r4, t32_putchar_cursor
    ldw  r5, [r4]

    ; BS (8): move left one cell, erase it, and leave cursor there.
    movi r1, 8
    xor  r1, r6, r1
    jz   r1, putchar_backspace

    ; LF (10)
    movi r1, 10
    xor  r1, r6, r1
    jz   r1, putchar_newline

    ; CR (13)
    movi r1, 13
    xor  r1, r6, r1
    jz   r1, putchar_carriage_return

    ; Ordinary character.
    stb  r6, [r5]
    addi r5, r5, 1

    ; If the cursor advanced one byte past the framebuffer, scroll.
    movi r1, 0x900007D0
    sub  r2, r5, r1
    jz   r2, putchar_scroll

putchar_store_cursor:
    stw  r5, [r4]
    mov  r0, r6
    ret

putchar_backspace:
    movi r1, 0x90000000
    sub  r2, r5, r1
    jz   r2, putchar_store_cursor
    subi r5, r5, 1
    movi r7, 32
    stb  r7, [r5]
    jmp  putchar_store_cursor

putchar_newline:
    ; Compute the beginning of the next row.
    movi r1, 0x90000000
    sub  r2, r5, r1
    movi r3, 80
    div  r2, r2, r3
    addi r2, r2, 1
    mul  r2, r2, r3
    add  r5, r1, r2

    movi r1, 0x900007D0
    sub  r2, r5, r1
    jz   r2, putchar_scroll
    jmp  putchar_store_cursor

putchar_carriage_return:
    ; Compute the beginning of the current row.
    movi r1, 0x90000000
    sub  r2, r5, r1
    movi r3, 80
    div  r2, r2, r3
    mul  r2, r2, r3
    add  r5, r1, r2
    jmp  putchar_store_cursor

putchar_scroll:
    ; Copy rows 1..24 to rows 0..23.
    movi r1, 0x90000050
    movi r2, 0x90000000
    movi r3, 1920

putchar_scroll_copy:
    ldb  r7, [r1]
    stb  r7, [r2]
    addi r1, r1, 1
    addi r2, r2, 1
    subi r3, r3, 1
    jnz  r3, putchar_scroll_copy

    ; Clear the final row.
    movi r2, 0x90000780
    movi r3, 80
    movi r7, 32

putchar_scroll_clear:
    stb  r7, [r2]
    addi r2, r2, 1
    subi r3, r3, 1
    jnz  r3, putchar_scroll_clear

    ; Cursor rests at column zero of the final row.
    movi r5, 0x90000780
    jmp  putchar_store_cursor

.section .data
t32_putchar_cursor:
    .word 0x90000500
