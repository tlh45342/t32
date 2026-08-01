; hex-to-string.s
;
; T32 algorithm validation: unsigned 32-bit value to fixed-width
; uppercase hexadecimal text.
;
; input:
;   r0 = unsigned 32-bit value
;   r1 = destination buffer (at least 9 bytes)
;
; output:
;   r2 = original destination pointer
;
; writes:
;   exactly eight uppercase hexadecimal digits
;   followed by a zero terminator
;
; clobbers:
;   r0-r6
;
; JZ/JNZ test a named register directly.

.org 0x00001000

.equ STACK_TOP, 0x0000F000
.equ PASS,      1
.equ FAIL,      0
.equ CASES,     5

    jmp start

host_guard_before:
    .byte 0xA5
host_output:
    .byte 0, 0, 0, 0, 0, 0, 0, 0, 0
host_guard_after:
    .byte 0x5A

output_1:
    .byte 0, 0, 0, 0, 0, 0, 0, 0, 0
output_2:
    .byte 0, 0, 0, 0, 0, 0, 0, 0, 0
output_3:
    .byte 0, 0, 0, 0, 0, 0, 0, 0, 0
output_4:
    .byte 0, 0, 0, 0, 0, 0, 0, 0, 0

expected_0:
    .byte '0', '0', '0', '0', '0', '0', '0', '0', 0
expected_a:
    .byte '0', '0', '0', '0', '0', '0', '0', 'A', 0
expected_1234:
    .byte '0', '0', '0', '0', '1', '2', '3', '4', 0
expected_mixed:
    .byte '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 0
expected_ones:
    .byte 'F', 'F', 'F', 'F', 'F', 'F', 'F', 'F', 0

hex_digits:
    .byte '0', '1', '2', '3', '4', '5', '6', '7'
    .byte '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'

start:
    movi r15, STACK_TOP
    movi r7, FAIL
    movi r14, 0

    movi r0, 0x00000000
    movi r1, host_output
    call hex_to_string
    movi r0, host_output
    movi r1, expected_0
    call string_equal
    mov  r8, r3
    addi r14, r14, 1

    movi r0, 0x0000000A
    movi r1, output_1
    call hex_to_string
    movi r0, output_1
    movi r1, expected_a
    call string_equal
    mov  r9, r3
    addi r14, r14, 1

    movi r0, 0x00001234
    movi r1, output_2
    call hex_to_string
    movi r0, output_2
    movi r1, expected_1234
    call string_equal
    mov  r10, r3
    addi r14, r14, 1

    movi r0, 0x89ABCDEF
    movi r1, output_3
    call hex_to_string
    movi r0, output_3
    movi r1, expected_mixed
    call string_equal
    mov  r11, r3
    addi r14, r14, 1

    movi r0, 0xFFFFFFFF
    movi r1, output_4
    call hex_to_string
    mov  r13, r2
    movi r0, output_4
    movi r1, expected_ones
    call string_equal
    mov  r12, r3
    addi r14, r14, 1

    jnz  r8, guest_fail
    jnz  r9, guest_fail
    jnz  r10, guest_fail
    jnz  r11, guest_fail
    jnz  r12, guest_fail

    movi r6, CASES
    xor  r6, r14, r6
    jnz  r6, guest_fail

    movi r6, STACK_TOP
    xor  r6, r15, r6
    jnz  r6, guest_fail

    movi r6, output_4
    xor  r6, r13, r6
    jnz  r6, guest_fail

    movi r0, host_guard_before
    ldb  r4, [r0]
    movi r5, 0xA5
    xor  r6, r4, r5
    jnz  r6, guest_fail

    movi r0, host_guard_after
    ldb  r4, [r0]
    movi r5, 0x5A
    xor  r6, r4, r5
    jnz  r6, guest_fail

    movi r7, PASS
    halt

guest_fail:
    movi r7, FAIL
    halt

hex_to_string:
    mov  r2, r1
    movi r3, 8
    movi r6, 28

hex_loop:
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
    jnz  r3, hex_loop

    movi r4, 0
    stb  r4, [r1]
    ret

string_equal:
string_equal_loop:
    ldb  r4, [r0]
    ldb  r5, [r1]

    xor  r6, r4, r5
    jnz  r6, string_different

    jz   r4, strings_equal

    addi r0, r0, 1
    addi r1, r1, 1
    jmp  string_equal_loop

strings_equal:
    movi r3, 0
    ret

string_different:
    movi r3, 1
    ret
