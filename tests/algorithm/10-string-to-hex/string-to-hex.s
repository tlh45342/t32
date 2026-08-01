; string-to-hex.s
;
; T32 algorithm validation: fixed-width hexadecimal text to uint32.
;
; string_to_hex contract
; ----------------------
; input:
;   r0 = pointer to exactly eight hexadecimal digits followed by zero
;
; output:
;   r1 = parsed unsigned 32-bit value
;   r2 = status: 0 success, 1 invalid input
;
; accepted:
;   '0' through '9'
;   'A' through 'F'
;   'a' through 'f'
;
; rejected:
;   fewer than eight digits
;   more than eight digits
;   any non-hexadecimal character
;
; On failure, r1 is returned as zero.
;
; clobbers:
;   r0-r9
;
; Branch model:
;   JZ/JNZ test a named general-purpose register directly.

.org 0x00001000

.equ STACK_TOP, 0x0000F000
.equ PASS,      1
.equ FAIL,      0
.equ CASES,     8

    jmp start

text_zero:
    .byte '0', '0', '0', '0', '0', '0', '0', '0', 0

text_a:
    .byte '0', '0', '0', '0', '0', '0', '0', 'A', 0

text_1234:
    .byte '0', '0', '0', '0', '1', '2', '3', '4', 0

text_mixed:
    .byte '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 0

text_lower:
    .byte 'd', 'e', 'a', 'd', 'b', 'e', 'e', 'f', 0

text_ones:
    .byte 'F', 'F', 'F', 'F', 'F', 'F', 'F', 'F', 0

text_invalid:
    .byte '1', '2', '3', '4', '5', '6', '7', 'G', 0

text_short:
    .byte '1', '2', '3', '4', 0

; A parallel character/value table avoids requiring relational branches.
; The same index into hex_values yields the numeric nibble.

hex_chars:
    .byte '0', '1', '2', '3', '4', '5', '6', '7'
    .byte '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    .byte 'a', 'b', 'c', 'd', 'e', 'f'

hex_values:
    .byte 0, 1, 2, 3, 4, 5, 6, 7
    .byte 8, 9, 10, 11, 12, 13, 14, 15
    .byte 10, 11, 12, 13, 14, 15

start:
    movi r15, STACK_TOP
    movi r7, FAIL
    movi r14, 0

    ; Case 1: zero.
    movi r0, text_zero
    call string_to_hex
    jnz  r2, guest_fail
    movi r6, 0x00000000
    xor  r6, r1, r6
    jnz  r6, guest_fail
    addi r14, r14, 1

    ; Case 2: one alphabetic digit.
    movi r0, text_a
    call string_to_hex
    jnz  r2, guest_fail
    movi r6, 0x0000000A
    xor  r6, r1, r6
    jnz  r6, guest_fail
    addi r14, r14, 1

    ; Case 3: leading zeroes.
    movi r0, text_1234
    call string_to_hex
    jnz  r2, guest_fail
    movi r6, 0x00001234
    xor  r6, r1, r6
    jnz  r6, guest_fail
    addi r14, r14, 1

    ; Case 4: mixed numeric and uppercase digits.
    movi r0, text_mixed
    call string_to_hex
    jnz  r2, guest_fail
    movi r6, 0x89ABCDEF
    xor  r6, r1, r6
    jnz  r6, guest_fail
    addi r14, r14, 1

    ; Case 5: lowercase digits.
    movi r0, text_lower
    call string_to_hex
    jnz  r2, guest_fail
    movi r6, 0xDEADBEEF
    xor  r6, r1, r6
    jnz  r6, guest_fail
    mov  r12, r1
    addi r14, r14, 1

    ; Case 6: all bits set.
    movi r0, text_ones
    call string_to_hex
    jnz  r2, guest_fail
    movi r6, 0xFFFFFFFF
    xor  r6, r1, r6
    jnz  r6, guest_fail
    addi r14, r14, 1

    ; Case 7: invalid digit is rejected and result is zero.
    movi r0, text_invalid
    call string_to_hex
    movi r6, 1
    xor  r6, r2, r6
    jnz  r6, guest_fail
    jnz  r1, guest_fail
    addi r14, r14, 1

    ; Case 8: short input is rejected and result is zero.
    movi r0, text_short
    call string_to_hex
    mov  r13, r2
    movi r6, 1
    xor  r6, r2, r6
    jnz  r6, guest_fail
    jnz  r1, guest_fail
    addi r14, r14, 1

    movi r6, CASES
    xor  r6, r14, r6
    jnz  r6, guest_fail

    movi r6, STACK_TOP
    xor  r6, r15, r6
    jnz  r6, guest_fail

    movi r7, PASS
    halt

guest_fail:
    movi r7, FAIL
    halt

; ------------------------------------------------------------
; string_to_hex
; ------------------------------------------------------------

string_to_hex:
    movi r1, 0
    movi r2, 1
    movi r3, 8

parse_digit:
    ldb  r4, [r0]
    jz   r4, parse_invalid

    movi r5, hex_chars
    movi r6, hex_values
    movi r7, 22

scan_table:
    ldb  r8, [r5]
    xor  r9, r4, r8
    jz   r9, digit_found

    addi r5, r5, 1
    addi r6, r6, 1
    subi r7, r7, 1
    jnz  r7, scan_table

    jmp  parse_invalid

digit_found:
    ldb  r8, [r6]

    movi r9, 4
    shl  r1, r1, r9
    or   r1, r1, r8

    addi r0, r0, 1
    subi r3, r3, 1
    jnz  r3, parse_digit

    ; Exactly eight digits are required.
    ldb  r4, [r0]
    jnz  r4, parse_invalid

    movi r2, 0
    ret

parse_invalid:
    movi r1, 0
    movi r2, 1
    ret
