; libt32 string_to_hex
;
; r0 = pointer to exactly eight hex digits followed by zero
; r1 = parsed uint32
; r2 = status: 0 success, 1 invalid
;
; Accepts uppercase and lowercase.
; clobbers r0-r9
;
string_to_hex:
    movi r1, 0
    movi r2, 1
    movi r3, 8
string_to_hex_digit:
    ldb  r4, [r0]
    jz   r4, string_to_hex_invalid
    movi r5, hex_chars
    movi r6, hex_values
    movi r7, 22
string_to_hex_scan:
    ldb  r8, [r5]
    xor  r9, r4, r8
    jz   r9, string_to_hex_found
    addi r5, r5, 1
    addi r6, r6, 1
    subi r7, r7, 1
    jnz  r7, string_to_hex_scan
    jmp  string_to_hex_invalid
string_to_hex_found:
    ldb  r8, [r6]
    movi r9, 4
    shl  r1, r1, r9
    or   r1, r1, r8
    addi r0, r0, 1
    subi r3, r3, 1
    jnz  r3, string_to_hex_digit
    ldb  r4, [r0]
    jnz  r4, string_to_hex_invalid
    movi r2, 0
    ret
string_to_hex_invalid:
    movi r1, 0
    movi r2, 1
    ret

hex_chars:
    .byte '0', '1', '2', '3', '4', '5', '6', '7'
    .byte '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    .byte 'a', 'b', 'c', 'd', 'e', 'f'
hex_values:
    .byte 0, 1, 2, 3, 4, 5, 6, 7
    .byte 8, 9, 10, 11, 12, 13, 14, 15
    .byte 10, 11, 12, 13, 14, 15
