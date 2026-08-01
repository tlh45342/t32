; strncpy.s
;
; T32 algorithm validation: libc-style strncpy.
;
; strncpy contract
; ----------------
; input:
;   r0 = destination pointer
;   r1 = source pointer
;   r2 = byte count
;
; output:
;   r3 = original destination pointer
;
; behavior:
;   copies at most count bytes
;   if source terminates early, remaining bytes are padded with zero
;   if source length is at least count, no terminator is guaranteed
;   count == 0 performs no memory access
;
; clobbers:
;   r0-r6
;
; Branch model:
;   JZ/JNZ test a named general-purpose register directly.

.org 0x00001000

.equ STACK_TOP, 0x0000F000
.equ PASS,      1
.equ FAIL,      0
.equ CASES,     6

    jmp start

source_empty:
    .byte 0

source_short:
    .byte 'A', 'B', 0

source_exact:
    .byte 'T', '3', '2', '!', 0

source_long:
    .byte 'F', 'o', 'u', 'n', 'd', 'r', 'y', 0

source_one:
    .byte 'Z', 0

; Case 1: count zero.
zero_guard_before:
    .byte 0xA1
zero_destination:
    .byte 0x11, 0x22, 0x33, 0x44
zero_guard_after:
    .byte 0xA2

zero_expected:
    .byte 0x11, 0x22, 0x33, 0x44

; Case 2: empty source pads entire range.
empty_guard_before:
    .byte 0xB1
empty_destination:
    .byte 0xCC, 0xCC, 0xCC, 0xCC
empty_guard_after:
    .byte 0xB2

empty_expected:
    .byte 0, 0, 0, 0

; Case 3: short source pads remaining bytes.
short_guard_before:
    .byte 0xC1
short_destination:
    .byte 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC
short_guard_after:
    .byte 0xC2

short_expected:
    .byte 'A', 'B', 0, 0, 0, 0

; Case 4: exact count copies exactly count bytes, no extra terminator.
exact_guard_before:
    .byte 0xD1
exact_destination:
    .byte 0xCC, 0xCC, 0xCC, 0xCC, 0x7E
exact_guard_after:
    .byte 0xD2

exact_expected:
    .byte 'T', '3', '2', '!', 0x7E

; Case 5: long source truncates to count and does not terminate.
long_guard_before:
    .byte 0xE1
long_destination:
    .byte 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x7F
long_guard_after:
    .byte 0xE2

long_expected:
    .byte 'F', 'o', 'u', 'n', 'd', 0x7F

; Case 6: one-byte count copies one byte only.
one_guard_before:
    .byte 0xF1
one_destination:
    .byte 0xCC, 0x55
one_guard_after:
    .byte 0xF2

one_expected:
    .byte 'Z', 0x55

start:
    movi r15, STACK_TOP
    movi r7, FAIL
    movi r14, 0

    ; Case 1: count zero.
    movi r0, zero_destination
    movi r1, source_long
    movi r2, 0
    call strncpy
    mov  r8, r3
    addi r14, r14, 1

    ; Case 2: empty source pads four bytes.
    movi r0, empty_destination
    movi r1, source_empty
    movi r2, 4
    call strncpy
    mov  r9, r3
    addi r14, r14, 1

    ; Case 3: short source pads remaining bytes.
    movi r0, short_destination
    movi r1, source_short
    movi r2, 6
    call strncpy
    mov  r10, r3
    addi r14, r14, 1

    ; Case 4: source length equals count.
    movi r0, exact_destination
    movi r1, source_exact
    movi r2, 4
    call strncpy
    mov  r11, r3
    addi r14, r14, 1

    ; Case 5: source longer than count.
    movi r0, long_destination
    movi r1, source_long
    movi r2, 5
    call strncpy
    mov  r12, r3
    addi r14, r14, 1

    ; Case 6: one-byte copy.
    movi r0, one_destination
    movi r1, source_one
    movi r2, 1
    call strncpy
    mov  r13, r3
    addi r14, r14, 1

    ; Validate returned destination pointers.
    movi r6, zero_destination
    xor  r6, r8, r6
    jnz  r6, guest_fail

    movi r6, empty_destination
    xor  r6, r9, r6
    jnz  r6, guest_fail

    movi r6, short_destination
    xor  r6, r10, r6
    jnz  r6, guest_fail

    movi r6, exact_destination
    xor  r6, r11, r6
    jnz  r6, guest_fail

    movi r6, long_destination
    xor  r6, r12, r6
    jnz  r6, guest_fail

    movi r6, one_destination
    xor  r6, r13, r6
    jnz  r6, guest_fail

    ; Validate destination contents.
    movi r0, zero_destination
    movi r1, zero_expected
    movi r2, 4
    call memory_equal
    jnz  r3, guest_fail

    movi r0, empty_destination
    movi r1, empty_expected
    movi r2, 4
    call memory_equal
    jnz  r3, guest_fail

    movi r0, short_destination
    movi r1, short_expected
    movi r2, 6
    call memory_equal
    jnz  r3, guest_fail

    movi r0, exact_destination
    movi r1, exact_expected
    movi r2, 5
    call memory_equal
    jnz  r3, guest_fail

    movi r0, long_destination
    movi r1, long_expected
    movi r2, 6
    call memory_equal
    jnz  r3, guest_fail

    movi r0, one_destination
    movi r1, one_expected
    movi r2, 2
    call memory_equal
    jnz  r3, guest_fail

    ; Validate all guard pairs.
    movi r0, zero_guard_before
    movi r1, 0xA1
    movi r2, zero_guard_after
    movi r3, 0xA2
    call guards_match
    jnz  r4, guest_fail

    movi r0, empty_guard_before
    movi r1, 0xB1
    movi r2, empty_guard_after
    movi r3, 0xB2
    call guards_match
    jnz  r4, guest_fail

    movi r0, short_guard_before
    movi r1, 0xC1
    movi r2, short_guard_after
    movi r3, 0xC2
    call guards_match
    jnz  r4, guest_fail

    movi r0, exact_guard_before
    movi r1, 0xD1
    movi r2, exact_guard_after
    movi r3, 0xD2
    call guards_match
    jnz  r4, guest_fail

    movi r0, long_guard_before
    movi r1, 0xE1
    movi r2, long_guard_after
    movi r3, 0xE2
    call guards_match
    jnz  r4, guest_fail

    movi r0, one_guard_before
    movi r1, 0xF1
    movi r2, one_guard_after
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
; strncpy
; ------------------------------------------------------------

strncpy:
    mov  r3, r0
    jz   r2, strncpy_done

strncpy_copy:
    ldb  r4, [r1]
    stb  r4, [r0]

    addi r0, r0, 1
    addi r1, r1, 1
    subi r2, r2, 1

    jz   r2, strncpy_done
    jnz  r4, strncpy_copy

    ; Source terminated before count. Pad remaining bytes with zero.
    movi r5, 0

strncpy_pad:
    stb  r5, [r0]
    addi r0, r0, 1
    subi r2, r2, 1
    jnz  r2, strncpy_pad

strncpy_done:
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

; ------------------------------------------------------------
; guards_match
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
