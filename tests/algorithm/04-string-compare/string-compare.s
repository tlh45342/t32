; 03-string-compare

.org 0x00001000

.equ STACK_TOP, 0x0000F000

start:
    movi r15, STACK_TOP
    movi r7, 1
    movi r14, 0

    movi r0, string_empty_a
    movi r1, string_empty_b
    call string_equal
    mov  r8, r2
    movi r3, 0
    xor  r4, r2, r3
    jnz  r4, fail
    addi r14, r14, 1

    movi r0, string_a_1
    movi r1, string_a_2
    call string_equal
    mov  r9, r2
    movi r3, 0
    xor  r4, r2, r3
    jnz  r4, fail
    addi r14, r14, 1

    movi r0, string_hello_1
    movi r1, string_hello_2
    call string_equal
    mov  r10, r2
    movi r3, 0
    xor  r4, r2, r3
    jnz  r4, fail
    addi r14, r14, 1

    movi r0, string_hello
    movi r1, string_hellx
    call string_equal
    mov  r11, r2
    movi r3, 1
    xor  r4, r2, r3
    jnz  r4, fail
    addi r14, r14, 1

    movi r0, string_abc
    movi r1, string_abcd
    call string_equal
    mov  r12, r2
    movi r3, 1
    xor  r4, r2, r3
    jnz  r4, fail
    addi r14, r14, 1

    movi r0, string_abcd
    movi r1, string_abc
    call string_equal
    mov  r13, r2
    movi r3, 1
    xor  r4, r2, r3
    jnz  r4, fail
    addi r14, r14, 1

pass:
    movi r7, 0
    halt

fail:
    halt

string_equal:
compare_loop:
    ldb  r3, [r0]
    ldb  r4, [r1]
    xor  r2, r3, r4
    jnz  r2, compare_not_equal
    jz   r3, compare_equal
    addi r0, r0, 1
    addi r1, r1, 1
    jmp  compare_loop

compare_equal:
    movi r2, 0
    ret

compare_not_equal:
    movi r2, 1
    ret

string_empty_a:
    .byte 0
string_empty_b:
    .byte 0
string_a_1:
    .ascii "A"
    .byte 0
string_a_2:
    .ascii "A"
    .byte 0
string_hello_1:
    .ascii "HELLO"
    .byte 0
string_hello_2:
    .ascii "HELLO"
    .byte 0
string_hello:
    .ascii "HELLO"
    .byte 0
string_hellx:
    .ascii "HELLX"
    .byte 0
string_abc:
    .ascii "ABC"
    .byte 0
string_abcd:
    .ascii "ABCD"
    .byte 0
