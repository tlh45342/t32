; memmove.s
;
; T32 algorithm validation: libc-style memmove.
;
; memmove contract
; ----------------
; input:
;   r0 = destination pointer
;   r1 = source pointer
;   r2 = byte count
;
; output:
;   r3 = original destination pointer
;
; clobbers:
;   r0-r6
;
; behavior:
;   copies exactly count bytes
;   safely handles overlapping source and destination regions
;   count == 0 performs no memory access
;   destination == source performs no memory access
;
; Direction rule:
;   copy backward only when destination lies inside:
;
;       source + 1 ... source + count - 1
;
;   otherwise copy forward.
;
; This rule is detected with explicit pointer equality checks and does not
; require flag-consuming relational branches.
;
; Branch model:
;   JZ/JNZ test a named general-purpose register directly.

.org 0x00001000

.equ STACK_TOP, 0x0000F000
.equ PASS,      1
.equ FAIL,      0
.equ CASES,     6

    jmp start

; ------------------------------------------------------------
; Case 1: zero-length operation.
; ------------------------------------------------------------

zero_guard_before:
    .byte 0xA1
zero_buffer:
    .byte 0x10, 0x20, 0x30, 0x40
zero_guard_after:
    .byte 0xA2

zero_expected:
    .byte 0x10, 0x20, 0x30, 0x40

; ------------------------------------------------------------
; Case 2: destination equals source.
; ------------------------------------------------------------

same_guard_before:
    .byte 0xB1
same_buffer:
    .byte 'T', '3', '2', '!'
same_guard_after:
    .byte 0xB2

same_expected:
    .byte 'T', '3', '2', '!'

; ------------------------------------------------------------
; Case 3: non-overlapping copy.
; ------------------------------------------------------------

plain_source:
    .byte 0x11, 0x22, 0x33, 0x44
    .byte 0x55, 0x66, 0x77, 0x88

plain_guard_before:
    .byte 0xC1
plain_destination:
    .byte 0, 0, 0, 0, 0, 0, 0, 0
plain_guard_after:
    .byte 0xC2

plain_expected:
    .byte 0x11, 0x22, 0x33, 0x44
    .byte 0x55, 0x66, 0x77, 0x88

; ------------------------------------------------------------
; Case 4: overlapping copy where destination is below source.
;
; memmove(buffer, buffer + 2, 8)
; ------------------------------------------------------------

forward_guard_before:
    .byte 0xD1
forward_buffer:
    .byte 'A', 'B', 'C', 'D', 'E', 'F'
    .byte 'G', 'H', 'I', 'J', 'K', 'L'
forward_guard_after:
    .byte 0xD2

forward_expected:
    .byte 'C', 'D', 'E', 'F', 'G', 'H'
    .byte 'I', 'J', 'I', 'J', 'K', 'L'

; ------------------------------------------------------------
; Case 5: overlapping copy where destination is inside source.
;
; memmove(buffer + 2, buffer, 8)
; This must copy backward.
; ------------------------------------------------------------

backward_guard_before:
    .byte 0xE1
backward_buffer:
    .byte 'A', 'B', 'C', 'D', 'E', 'F'
    .byte 'G', 'H', 'I', 'J', 'K', 'L'
backward_guard_after:
    .byte 0xE2

backward_expected:
    .byte 'A', 'B', 'A', 'B', 'C', 'D'
    .byte 'E', 'F', 'G', 'H', 'K', 'L'

; ------------------------------------------------------------
; Case 6: one-byte non-overlapping copy.
; ------------------------------------------------------------

one_source:
    .byte 0x5A

one_guard_before:
    .byte 0xF1
one_destination:
    .byte 0x00
one_guard_after:
    .byte 0xF2

one_expected:
    .byte 0x5A

start:
    movi r15, STACK_TOP
    movi r7, FAIL
    movi r14, 0

    ; Case 1: count zero.
    movi r0, zero_buffer
    movi r1, plain_source
    movi r2, 0
    call memmove
    mov  r8, r3
    addi r14, r14, 1

    ; Case 2: identical pointers.
    movi r0, same_buffer
    movi r1, same_buffer
    movi r2, 4
    call memmove
    mov  r9, r3
    addi r14, r14, 1

    ; Case 3: non-overlapping regions.
    movi r0, plain_destination
    movi r1, plain_source
    movi r2, 8
    call memmove
    mov  r10, r3
    addi r14, r14, 1

    ; Case 4: overlap, destination below source. Copy forward.
    movi r0, forward_buffer
    movi r1, forward_buffer
    addi r1, r1, 2
    movi r2, 8
    call memmove
    mov  r11, r3
    addi r14, r14, 1

    ; Case 5: overlap, destination inside source. Copy backward.
    movi r0, backward_buffer
    addi r0, r0, 2
    movi r1, backward_buffer
    movi r2, 8
    call memmove
    mov  r12, r3
    addi r14, r14, 1

    ; Case 6: one byte.
    movi r0, one_destination
    movi r1, one_source
    movi r2, 1
    call memmove
    mov  r13, r3
    addi r14, r14, 1

    ; ------------------------------------------------------------
    ; Validate returned destination pointers.
    ; ------------------------------------------------------------

    movi r6, zero_buffer
    xor  r6, r8, r6
    jnz  r6, guest_fail

    movi r6, same_buffer
    xor  r6, r9, r6
    jnz  r6, guest_fail

    movi r6, plain_destination
    xor  r6, r10, r6
    jnz  r6, guest_fail

    movi r6, forward_buffer
    xor  r6, r11, r6
    jnz  r6, guest_fail

    movi r6, backward_buffer
    addi r6, r6, 2
    xor  r6, r12, r6
    jnz  r6, guest_fail

    movi r6, one_destination
    xor  r6, r13, r6
    jnz  r6, guest_fail

    ; ------------------------------------------------------------
    ; Validate memory contents.
    ; ------------------------------------------------------------

    movi r0, zero_buffer
    movi r1, zero_expected
    movi r2, 4
    call memory_equal
    jnz  r3, guest_fail

    movi r0, same_buffer
    movi r1, same_expected
    movi r2, 4
    call memory_equal
    jnz  r3, guest_fail

    movi r0, plain_destination
    movi r1, plain_expected
    movi r2, 8
    call memory_equal
    jnz  r3, guest_fail

    movi r0, forward_buffer
    movi r1, forward_expected
    movi r2, 12
    call memory_equal
    jnz  r3, guest_fail

    movi r0, backward_buffer
    movi r1, backward_expected
    movi r2, 12
    call memory_equal
    jnz  r3, guest_fail

    movi r0, one_destination
    movi r1, one_expected
    movi r2, 1
    call memory_equal
    jnz  r3, guest_fail

    ; ------------------------------------------------------------
    ; Validate all guard pairs.
    ; ------------------------------------------------------------

    movi r0, zero_guard_before
    movi r1, 0xA1
    movi r2, zero_guard_after
    movi r3, 0xA2
    call guards_match
    jnz  r4, guest_fail

    movi r0, same_guard_before
    movi r1, 0xB1
    movi r2, same_guard_after
    movi r3, 0xB2
    call guards_match
    jnz  r4, guest_fail

    movi r0, plain_guard_before
    movi r1, 0xC1
    movi r2, plain_guard_after
    movi r3, 0xC2
    call guards_match
    jnz  r4, guest_fail

    movi r0, forward_guard_before
    movi r1, 0xD1
    movi r2, forward_guard_after
    movi r3, 0xD2
    call guards_match
    jnz  r4, guest_fail

    movi r0, backward_guard_before
    movi r1, 0xE1
    movi r2, backward_guard_after
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
; memmove
; ------------------------------------------------------------

memmove:
    mov  r3, r0

    ; Nothing to copy.
    jz   r2, memmove_done

    ; Identical regions need no memory access.
    xor  r4, r0, r1
    jz   r4, memmove_done

    ; Determine whether destination begins inside the source range.
    ;
    ; Scan:
    ;   source + 1 ... source + count - 1
    ;
    ; If destination matches one of those addresses, forward copying would
    ; overwrite unread source bytes, so use the backward loop.

    mov  r4, r1
    addi r4, r4, 1

    mov  r5, r2
    subi r5, r5, 1
    jz   r5, copy_forward

overlap_scan:
    xor  r6, r0, r4
    jz   r6, copy_backward

    addi r4, r4, 1
    subi r5, r5, 1
    jnz  r5, overlap_scan

copy_forward:
    ldb  r4, [r1]
    stb  r4, [r0]

    addi r0, r0, 1
    addi r1, r1, 1
    subi r2, r2, 1
    jnz  r2, copy_forward
    ret

copy_backward:
    ; Move each pointer to the final byte in its region.
    add  r0, r0, r2
    subi r0, r0, 1

    add  r1, r1, r2
    subi r1, r1, 1

copy_backward_loop:
    ldb  r4, [r1]
    stb  r4, [r0]

    subi r0, r0, 1
    subi r1, r1, 1
    subi r2, r2, 1
    jnz  r2, copy_backward_loop

memmove_done:
    ret

; ------------------------------------------------------------
; memory_equal
;
; input:
;   r0 = left region
;   r1 = right region
;   r2 = count
;
; output:
;   r3 = 0 equal, 1 different
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
;
; input:
;   r0 = first guard address
;   r1 = expected first byte
;   r2 = second guard address
;   r3 = expected second byte
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
