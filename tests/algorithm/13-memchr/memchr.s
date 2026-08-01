; memchr.s
;
; T32 algorithm validation: libc-style memchr.
;
; input:
;   r0 = memory pointer
;   r1 = search value (only low 8 bits are used)
;   r2 = byte count
;
; output:
;   r3 = pointer to first matching byte, or 0 when absent
;
; clobbers: r0, r2, r4-r6

.org 0x00001000

.equ STACK_TOP, 0x0000F000
.equ PASS,      1
.equ FAIL,      0
.equ CASES,     6

    jmp start

guard_before:
    .byte 0xA5
search_data:
    .byte 0x10, 0x20, 0x30, 0x40
    .byte 0x50, 0x60, 0x70, 0x80
    .byte 0x90, 0xA0, 0xB0, 0xC0
    .byte 0xD0, 0xE0, 0xF0, 0x30
guard_after:
    .byte 0x5A

expected_data:
    .byte 0x10, 0x20, 0x30, 0x40
    .byte 0x50, 0x60, 0x70, 0x80
    .byte 0x90, 0xA0, 0xB0, 0xC0
    .byte 0xD0, 0xE0, 0xF0, 0x30

start:
    movi r15, STACK_TOP
    movi r7, FAIL
    movi r14, 0

    ; Case 1: first byte matches.
    movi r0, search_data
    movi r1, 0x10
    movi r2, 16
    call memchr
    mov  r8, r3
    addi r14, r14, 1

    ; Case 2: middle byte matches.
    movi r0, search_data
    movi r1, 0x80
    movi r2, 16
    call memchr
    mov  r9, r3
    addi r14, r14, 1

    ; Case 3: duplicate value returns first occurrence.
    movi r0, search_data
    movi r1, 0x30
    movi r2, 16
    call memchr
    mov  r10, r3
    addi r14, r14, 1

    ; Case 4: restricted range finds final byte.
    movi r0, search_data
    addi r0, r0, 3
    movi r1, 0x30
    movi r2, 13
    call memchr
    mov  r11, r3
    addi r14, r14, 1

    ; Case 5: absent value returns zero.
    movi r0, search_data
    movi r1, 0x55
    movi r2, 16
    call memchr
    mov  r12, r3
    addi r14, r14, 1

    ; Case 6: low byte is used; zero count prevents memory access.
    movi r0, guard_before
    movi r1, 0x123456A5
    movi r2, 0
    call memchr
    mov  r13, r3
    addi r14, r14, 1

    ; Validate returned pointers.
    movi r6, search_data
    xor  r6, r8, r6
    jnz  r6, guest_fail

    movi r6, search_data
    addi r6, r6, 7
    xor  r6, r9, r6
    jnz  r6, guest_fail

    movi r6, search_data
    addi r6, r6, 2
    xor  r6, r10, r6
    jnz  r6, guest_fail

    movi r6, search_data
    addi r6, r6, 15
    xor  r6, r11, r6
    jnz  r6, guest_fail

    jnz  r12, guest_fail
    jnz  r13, guest_fail

    ; Source memory must remain unchanged.
    movi r0, search_data
    movi r1, expected_data
    movi r2, 16
    call memory_equal
    jnz  r3, guest_fail

    ; Guard bytes must remain unchanged.
    movi r0, guard_before
    ldb  r4, [r0]
    movi r5, 0xA5
    xor  r6, r4, r5
    jnz  r6, guest_fail

    movi r0, guard_after
    ldb  r4, [r0]
    movi r5, 0x5A
    xor  r6, r4, r5
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
; memchr
; ------------------------------------------------------------

memchr:
    jz   r2, memchr_not_found

    ; Mask the requested value once before the loop.
    movi r5, 0xFF
    and  r1, r1, r5

memchr_loop:
    ldb  r4, [r0]
    xor  r6, r4, r1
    jz   r6, memchr_found

    addi r0, r0, 1
    subi r2, r2, 1
    jnz  r2, memchr_loop

memchr_not_found:
    movi r3, 0
    ret

memchr_found:
    mov  r3, r0
    ret

; ------------------------------------------------------------
; memory_equal
; ------------------------------------------------------------

memory_equal:
    jz   r2, memory_equal_match

memory_equal_loop:
    ldb  r4, [r0]
    ldb  r5, [r1]
    xor  r6, r4, r5
    jnz  r6, memory_equal_different

    addi r0, r0, 1
    addi r1, r1, 1
    subi r2, r2, 1
    jnz  r2, memory_equal_loop

memory_equal_match:
    movi r3, 0
    ret

memory_equal_different:
    movi r3, 1
    ret
