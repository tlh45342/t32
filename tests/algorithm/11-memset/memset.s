; memset.s
;
; T32 algorithm validation: libc-style memset.
;
; memset contract
; ---------------
; input:
;   r0 = destination pointer
;   r1 = fill value (only low 8 bits are used)
;   r2 = byte count
;
; output:
;   r3 = original destination pointer
;
; clobbers:
;   r0, r2, r4-r6
;
; behavior:
;   writes count bytes
;   count == 0 performs no memory access
;   source value is preserved
;   destination guard bytes remain unchanged
;
; Branch model:
;   JZ/JNZ test a named register directly.

.org 0x00001000

.equ STACK_TOP, 0x0000F000
.equ PASS,      1
.equ FAIL,      0
.equ CASES,     6

    jmp start

; Case 1: zero-length region surrounded by guards.
zero_guard_before:
    .byte 0xA1
zero_buffer:
    .byte 0x11, 0x22, 0x33, 0x44
zero_guard_after:
    .byte 0xA2

; Case 2: single byte.
one_guard_before:
    .byte 0xB1
one_buffer:
    .byte 0x00
one_guard_after:
    .byte 0xB2

; Case 3: sixteen bytes filled with zero.
zeroes_guard_before:
    .byte 0xC1
zeroes_buffer:
    .byte 0xFF, 0xFF, 0xFF, 0xFF
    .byte 0xFF, 0xFF, 0xFF, 0xFF
    .byte 0xFF, 0xFF, 0xFF, 0xFF
    .byte 0xFF, 0xFF, 0xFF, 0xFF
zeroes_guard_after:
    .byte 0xC2

; Case 4: sixteen bytes filled with 0xFF.
ones_guard_before:
    .byte 0xD1
ones_buffer:
    .byte 0x00, 0x00, 0x00, 0x00
    .byte 0x00, 0x00, 0x00, 0x00
    .byte 0x00, 0x00, 0x00, 0x00
    .byte 0x00, 0x00, 0x00, 0x00
ones_guard_after:
    .byte 0xD2

; Case 5: low byte of a 32-bit value is used.
low_guard_before:
    .byte 0xE1
low_buffer:
    .byte 0x00, 0x00, 0x00, 0x00
    .byte 0x00, 0x00, 0x00, 0x00
low_guard_after:
    .byte 0xE2

; Case 6: larger 32-byte fill for loop coverage and host inspection.
host_guard_before:
    .byte 0xF1
host_buffer:
    .byte 0, 0, 0, 0, 0, 0, 0, 0
    .byte 0, 0, 0, 0, 0, 0, 0, 0
    .byte 0, 0, 0, 0, 0, 0, 0, 0
    .byte 0, 0, 0, 0, 0, 0, 0, 0
host_guard_after:
    .byte 0xF2

start:
    movi r15, STACK_TOP
    movi r7, FAIL
    movi r14, 0

    ; Case 1: zero count does not touch memory and returns original pointer.
    movi r0, zero_buffer
    movi r1, 0xAA
    movi r2, 0
    call memset
    mov  r8, r3
    addi r14, r14, 1

    ; Case 2: one byte.
    movi r0, one_buffer
    movi r1, 0x5A
    movi r2, 1
    call memset
    mov  r9, r3
    addi r14, r14, 1

    ; Case 3: sixteen zero bytes.
    movi r0, zeroes_buffer
    movi r1, 0x00
    movi r2, 16
    call memset
    mov  r10, r3
    addi r14, r14, 1

    ; Case 4: sixteen 0xFF bytes.
    movi r0, ones_buffer
    movi r1, 0xFF
    movi r2, 16
    call memset
    mov  r11, r3
    addi r14, r14, 1

    ; Case 5: only low byte 0x78 is stored.
    movi r0, low_buffer
    movi r1, 0x12345678
    movi r2, 8
    call memset
    mov  r12, r3
    addi r14, r14, 1

    ; Case 6: 32-byte fill and preserve original source value.
    movi r0, host_buffer
    movi r1, 0xA5A5A53C
    movi r2, 32
    call memset
    mov  r13, r3
    addi r14, r14, 1

    ; ------------------------------------------------------------
    ; Validate returned destination pointers.
    ; ------------------------------------------------------------

    movi r6, zero_buffer
    xor  r6, r8, r6
    jnz  r6, guest_fail

    movi r6, one_buffer
    xor  r6, r9, r6
    jnz  r6, guest_fail

    movi r6, zeroes_buffer
    xor  r6, r10, r6
    jnz  r6, guest_fail

    movi r6, ones_buffer
    xor  r6, r11, r6
    jnz  r6, guest_fail

    movi r6, low_buffer
    xor  r6, r12, r6
    jnz  r6, guest_fail

    movi r6, host_buffer
    xor  r6, r13, r6
    jnz  r6, guest_fail

    ; Source value must remain unchanged.
    movi r6, 0xA5A5A53C
    xor  r6, r1, r6
    jnz  r6, guest_fail

    ; ------------------------------------------------------------
    ; Validate zero-length case remained unchanged.
    ; ------------------------------------------------------------

    movi r0, zero_guard_before
    ldb  r4, [r0]
    movi r5, 0xA1
    xor  r6, r4, r5
    jnz  r6, guest_fail

    movi r0, zero_buffer
    ldb  r4, [r0]
    movi r5, 0x11
    xor  r6, r4, r5
    jnz  r6, guest_fail

    addi r0, r0, 3
    ldb  r4, [r0]
    movi r5, 0x44
    xor  r6, r4, r5
    jnz  r6, guest_fail

    movi r0, zero_guard_after
    ldb  r4, [r0]
    movi r5, 0xA2
    xor  r6, r4, r5
    jnz  r6, guest_fail

    ; ------------------------------------------------------------
    ; Validate one-byte fill and guards.
    ; ------------------------------------------------------------

    movi r0, one_buffer
    ldb  r4, [r0]
    movi r5, 0x5A
    xor  r6, r4, r5
    jnz  r6, guest_fail

    movi r0, one_guard_before
    ldb  r4, [r0]
    movi r5, 0xB1
    xor  r6, r4, r5
    jnz  r6, guest_fail

    movi r0, one_guard_after
    ldb  r4, [r0]
    movi r5, 0xB2
    xor  r6, r4, r5
    jnz  r6, guest_fail

    ; ------------------------------------------------------------
    ; Validate full regions with a helper.
    ; ------------------------------------------------------------

    movi r0, zeroes_buffer
    movi r1, 0x00
    movi r2, 16
    call region_is_byte
    jnz  r3, guest_fail

    movi r0, ones_buffer
    movi r1, 0xFF
    movi r2, 16
    call region_is_byte
    jnz  r3, guest_fail

    movi r0, low_buffer
    movi r1, 0x78
    movi r2, 8
    call region_is_byte
    jnz  r3, guest_fail

    movi r0, host_buffer
    movi r1, 0x3C
    movi r2, 32
    call region_is_byte
    jnz  r3, guest_fail

    ; Validate representative guard pairs.
    movi r0, zeroes_guard_before
    movi r1, 0xC1
    movi r2, zeroes_guard_after
    movi r3, 0xC2
    call guards_match
    jnz  r4, guest_fail

    movi r0, ones_guard_before
    movi r1, 0xD1
    movi r2, ones_guard_after
    movi r3, 0xD2
    call guards_match
    jnz  r4, guest_fail

    movi r0, low_guard_before
    movi r1, 0xE1
    movi r2, low_guard_after
    movi r3, 0xE2
    call guards_match
    jnz  r4, guest_fail

    movi r0, host_guard_before
    movi r1, 0xF1
    movi r2, host_guard_after
    movi r3, 0xF2
    call guards_match
    jnz  r4, guest_fail

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
; memset
; ------------------------------------------------------------

memset:
    mov  r3, r0
    jz   r2, memset_done

memset_loop:
    stb  r1, [r0]
    addi r0, r0, 1
    subi r2, r2, 1
    jnz  r2, memset_loop

memset_done:
    ret

; ------------------------------------------------------------
; region_is_byte
;
; input:
;   r0 = region pointer
;   r1 = expected low byte
;   r2 = count
;
; output:
;   r3 = 0 match, 1 mismatch
; ------------------------------------------------------------

region_is_byte:
    jz   r2, region_match

region_check_loop:
    ldb  r4, [r0]
    xor  r5, r4, r1
    jnz  r5, region_mismatch

    addi r0, r0, 1
    subi r2, r2, 1
    jnz  r2, region_check_loop

region_match:
    movi r3, 0
    ret

region_mismatch:
    movi r3, 1
    ret

; ------------------------------------------------------------
; guards_match
;
; input:
;   r0 = first guard address
;   r1 = first expected byte
;   r2 = second guard address
;   r3 = second expected byte
;
; output:
;   r4 = 0 match, 1 mismatch
; ------------------------------------------------------------

guards_match:
    ldb  r5, [r0]
    xor  r6, r5, r1
    jnz  r6, guards_mismatch

    ldb  r5, [r2]
    xor  r6, r5, r3
    jnz  r6, guards_mismatch

    movi r4, 0
    ret

guards_mismatch:
    movi r4, 1
    ret
