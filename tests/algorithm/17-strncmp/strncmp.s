; strncmp.s
;
; T32 algorithm validation: libc-style strncmp.
;
; strncmp contract
; ----------------
; input:
;   r0 = pointer to left zero-terminated string
;   r1 = pointer to right zero-terminated string
;   r2 = maximum byte count
;
; output:
;   r3 = first unequal unsigned-byte difference: left - right
;        zero when equal within count bytes
;
; behavior:
;   compares at most count bytes
;   stops at first mismatch
;   stops when equal terminators are reached
;   count == 0 returns zero without reading memory
;
; clobbers:
;   r0-r2, r4-r6
;
; Branch model:
;   JZ/JNZ test a named general-purpose register directly.

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

prefix_a:
    .byte 'a', 'b', 'c', 'X', 0
prefix_b:
    .byte 'a', 'b', 'c', 'Y', 0

high_s:
    .byte 0xFF, 0
low_s:
    .byte 0x01, 0

start:
    movi r15, STACK_TOP
    movi r7, FAIL
    movi r14, 0

    ; Case 1: count zero returns equal without reading.
    movi r0, high_s
    movi r1, low_s
    movi r2, 0
    call strncmp
    mov  r8, r3
    addi r14, r14, 1

    ; Case 2: empty strings compare equal.
    movi r0, empty_a
    movi r1, empty_b
    movi r2, 8
    call strncmp
    mov  r9, r3
    addi r14, r14, 1

    ; Case 3: identical strings compare equal.
    movi r0, same_a
    movi r1, same_b
    movi r2, 16
    call strncmp
    mov  r10, r3
    addi r14, r14, 1

    ; Case 4: mismatch inside limit: 'c' - 'd' = -1.
    movi r0, abc
    movi r1, abd
    movi r2, 3
    call strncmp
    mov  r11, r3
    addi r14, r14, 1

    ; Case 5: mismatch beyond limit is ignored.
    movi r0, prefix_a
    movi r1, prefix_b
    movi r2, 3
    call strncmp
    mov  r12, r3
    addi r14, r14, 1

    ; Case 6: left terminates first within limit: 0 - 'c' = -99.
    movi r0, short_s
    movi r1, long_s
    movi r2, 3
    call strncmp
    mov  r13, r3
    addi r14, r14, 1

    ; Case 7: limit ends before shorter-string difference.
    movi r0, short_s
    movi r1, long_s
    movi r2, 2
    call strncmp
    mov  r6, r3
    addi r14, r14, 1
    jnz  r6, guest_fail

    ; Case 8: unsigned-byte difference: 0xFF - 0x01 = 254.
    movi r0, high_s
    movi r1, low_s
    movi r2, 1
    call strncmp
    mov  r6, r3
    addi r14, r14, 1

    movi r5, 254
    xor  r5, r6, r5
    jnz  r5, guest_fail

    ; Validate saved results.
    jnz  r8, guest_fail
    jnz  r9, guest_fail
    jnz  r10, guest_fail

    movi r6, 0xFFFFFFFF
    xor  r6, r11, r6
    jnz  r6, guest_fail

    jnz  r12, guest_fail

    movi r6, 0xFFFFFF9D
    xor  r6, r13, r6
    jnz  r6, guest_fail

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
; strncmp
; ------------------------------------------------------------

strncmp:
    jz   r2, strncmp_equal

strncmp_loop:
    ldb  r4, [r0]
    ldb  r5, [r1]

    xor  r6, r4, r5
    jnz  r6, strncmp_different

    ; Equal zero bytes mean both strings ended together.
    jz   r4, strncmp_equal

    addi r0, r0, 1
    addi r1, r1, 1
    subi r2, r2, 1
    jnz  r2, strncmp_loop

strncmp_equal:
    movi r3, 0
    ret

strncmp_different:
    sub  r3, r4, r5
    ret
