; strcmp.s
;
; T32 algorithm validation: libc-style strcmp.
;
; input:
;   r0 = pointer to left string
;   r1 = pointer to right string
;
; output:
;   r2 = first unequal unsigned byte difference: left - right
;        zero when equal
;
; clobbers:
;   r0, r1, r3-r5

.org 0x00001000

.equ STACK_TOP, 0x0000F000
.equ PASS,      1
.equ FAIL,      0
.equ CASES,     8

    jmp start

empty_a:
    .byte 0
empty_b:
    .byte 0
same_a:
    .byte 'F', 'o', 'u', 'n', 'd', 'r', 'y', 0
same_b:
    .byte 'F', 'o', 'u', 'n', 'd', 'r', 'y', 0
abc:
    .byte 'a', 'b', 'c', 0
abd:
    .byte 'a', 'b', 'd', 0
short_s:
    .byte 'a', 'b', 0
long_s:
    .byte 'a', 'b', 'c', 0
upper_s:
    .byte 'A', 0
lower_s:
    .byte 'a', 0
high_s:
    .byte 0xFF, 0
low_s:
    .byte 0x01, 0

start:
    movi r15, STACK_TOP
    movi r7, FAIL
    movi r14, 0

    movi r0, empty_a
    movi r1, empty_b
    call strcmp
    mov r8, r2
    addi r14, r14, 1

    movi r0, same_a
    movi r1, same_b
    call strcmp
    mov r9, r2
    addi r14, r14, 1

    movi r0, abc
    movi r1, abd
    call strcmp
    mov r10, r2
    addi r14, r14, 1

    movi r0, abd
    movi r1, abc
    call strcmp
    mov r11, r2
    addi r14, r14, 1

    movi r0, short_s
    movi r1, long_s
    call strcmp
    mov r12, r2
    addi r14, r14, 1

    movi r0, long_s
    movi r1, short_s
    call strcmp
    mov r13, r2
    addi r14, r14, 1

    movi r0, upper_s
    movi r1, lower_s
    call strcmp
    mov r6, r2
    addi r14, r14, 1
    movi r5, 0xFFFFFFE0
    xor r5, r6, r5
    jnz r5, guest_fail

    movi r0, high_s
    movi r1, low_s
    call strcmp
    mov r6, r2
    addi r14, r14, 1
    movi r5, 254
    xor r5, r6, r5
    jnz r5, guest_fail

    jnz r8, guest_fail
    jnz r9, guest_fail

    movi r6, 0xFFFFFFFF
    xor r6, r10, r6
    jnz r6, guest_fail

    movi r6, 1
    xor r6, r11, r6
    jnz r6, guest_fail

    movi r6, 0xFFFFFF9D
    xor r6, r12, r6
    jnz r6, guest_fail

    movi r6, 99
    xor r6, r13, r6
    jnz r6, guest_fail

    movi r6, CASES
    xor r6, r14, r6
    jnz r6, guest_fail

    movi r6, STACK_TOP
    xor r6, r15, r6
    jnz r6, guest_fail

    movi r7, PASS
    halt

guest_fail:
    movi r7, FAIL
    halt

strcmp:
strcmp_loop:
    ldb r3, [r0]
    ldb r4, [r1]

    xor r5, r3, r4
    jnz r5, strcmp_different

    jz r3, strcmp_equal

    addi r0, r0, 1
    addi r1, r1, 1
    jmp strcmp_loop

strcmp_different:
    sub r2, r3, r4
    ret

strcmp_equal:
    movi r2, 0
    ret
