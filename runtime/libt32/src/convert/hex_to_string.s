; libt32 hex_to_string -- T32 ABI 0.1
; Arguments: r0=value, r1=destination buffer (9 bytes)
; Returns:   r0=original destination
; Preserves: r8-r15
; Clobbers:  r1-r6
; Writes eight uppercase digits and a zero terminator.
.section .text
.global hex_to_string
hex_to_string:
    mov  r2, r1
    movi r3, 8
    movi r6, 28
hex_to_string_loop:
    shr  r4, r0, r6
    movi r5, 0x0F
    and  r4, r4, r5
    movi r5, hex_digits
    add  r5, r5, r4
    ldb  r4, [r5]
    stb  r4, [r1]
    addi r1, r1, 1
    movi r5, 4
    shl  r0, r0, r5
    subi r3, r3, 1
    jnz  r3, hex_to_string_loop
    movi r4, 0
    stb  r4, [r1]
    mov  r0, r2
    ret
.section .data
hex_digits:
    .byte '0', '1', '2', '3', '4', '5', '6', '7'
    .byte '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
