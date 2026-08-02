; libt32 string_to_hex -- T32 ABI 0.1
; Arguments: r0=exactly eight hex digits followed by zero
; Returns:   r0=parsed value, r1=status (0 success, 1 invalid)
; Preserves: r8-r15
; Clobbers:  r2-r7
.section .text
.global string_to_hex
string_to_hex:
    push r8
    push r9
    mov  r3, r0
    movi r2, 0
    movi r1, 1
    movi r4, 8
string_to_hex_digit:
    ldb  r5, [r3]
    jz   r5, string_to_hex_invalid
    movi r6, hex_chars
    movi r7, hex_values
    movi r8, 22
string_to_hex_scan:
    ldb  r9, [r6]
    xor  r9, r5, r9
    jz   r9, string_to_hex_found
    addi r6, r6, 1
    addi r7, r7, 1
    subi r8, r8, 1
    jnz  r8, string_to_hex_scan
    jmp  string_to_hex_invalid
string_to_hex_found:
    ldb  r9, [r7]
    movi r5, 4
    shl  r2, r2, r5
    or   r2, r2, r9
    addi r3, r3, 1
    subi r4, r4, 1
    jnz  r4, string_to_hex_digit
    ldb  r5, [r3]
    jnz  r5, string_to_hex_invalid
    mov  r0, r2
    movi r1, 0
    jmp  string_to_hex_return
string_to_hex_invalid:
    movi r0, 0
    movi r1, 1
string_to_hex_return:
    pop  r9
    pop  r8
    ret
.section .data
hex_chars:
    .byte '0', '1', '2', '3', '4', '5', '6', '7'
    .byte '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    .byte 'a', 'b', 'c', 'd', 'e', 'f'
hex_values:
    .byte 0, 1, 2, 3, 4, 5, 6, 7
    .byte 8, 9, 10, 11, 12, 13, 14, 15
    .byte 10, 11, 12, 13, 14, 15
