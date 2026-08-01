; string-copy.s
;
; T32 algorithm validation: zero-terminated string copy.
;
; string_copy contract
; --------------------
; input:
;   r0 = destination pointer
;   r1 = source pointer
;
; output:
;   r2 = original destination pointer
;
; clobbers:
;   r0-r3
;
; validation results:
;   r8-r12 = five successful copy cases
;   r13    = destination guard validation
;   r14    = number of completed cases
;   r15    = restored stack pointer
;   r7     = 1 PASS, 0 FAIL

.org 0x00001000

.equ STACK_TOP, 0x0000F000
.equ PASS,      1
.equ FAIL,      0
.equ CASES,     5

start:
    movi r15, STACK_TOP
    movi r7, FAIL
    movi r14, 0

    ; Case 1: copy an empty string, including its terminator.
    movi r0, dst_empty
    movi r1, src_empty
    call string_copy
    movi r4, dst_empty
    xor r5, r2, r4
    jnz r5, guest_fail
    movi r0, dst_empty
    movi r1, src_empty
    call string_equal
    mov r8, r3
    addi r14, r14, 1

    ; Case 2: copy a one-character string.
    movi r0, dst_one
    movi r1, src_one
    call string_copy
    movi r4, dst_one
    xor r5, r2, r4
    jnz r5, guest_fail
    movi r0, dst_one
    movi r1, src_one
    call string_equal
    mov r9, r3
    addi r14, r14, 1

    ; Case 3: copy a short machine name.
    movi r0, dst_t32
    movi r1, src_t32
    call string_copy
    movi r4, dst_t32
    xor r5, r2, r4
    jnz r5, guest_fail
    movi r0, dst_t32
    movi r1, src_t32
    call string_equal
    mov r10, r3
    addi r14, r14, 1

    ; Case 4: copy a normal word.
    movi r0, dst_foundry
    movi r1, src_foundry
    call string_copy
    movi r4, dst_foundry
    xor r5, r2, r4
    jnz r5, guest_fail
    movi r0, dst_foundry
    movi r1, src_foundry
    call string_equal
    mov r11, r3
    addi r14, r14, 1

    ; Case 5: copy a phrase with spaces into a guarded buffer.
    movi r0, dst_phrase
    movi r1, src_phrase
    call string_copy
    movi r4, dst_phrase
    xor r5, r2, r4
    jnz r5, guest_fail
    movi r0, dst_phrase
    movi r1, src_phrase
    call string_equal
    mov r12, r3
    addi r14, r14, 1

    ; Validate the bytes immediately before and after the phrase buffer.
    movi r13, PASS
    movi r0, dst_phrase_guard_before
    ldb r1, [r0]
    movi r2, 0xA5
    xor r3, r1, r2
    jnz r3, guard_fail

    movi r0, dst_phrase_guard_after
    ldb r1, [r0]
    movi r2, 0x5A
    xor r3, r1, r2
    jnz r3, guard_fail
    jmp guards_done

guard_fail:
    movi r13, FAIL

guards_done:
    ; Every case and both guards must report success.
    jz r8, guest_fail
    jz r9, guest_fail
    jz r10, guest_fail
    jz r11, guest_fail
    jz r12, guest_fail
    jz r13, guest_fail

    movi r0, CASES
    xor r1, r14, r0
    jnz r1, guest_fail

    movi r0, STACK_TOP
    xor r1, r15, r0
    jnz r1, guest_fail

    movi r7, PASS
    halt

guest_fail:
    movi r7, FAIL
    halt

; ------------------------------------------------------------
; string_copy
; ------------------------------------------------------------

string_copy:
    mov r2, r0

copy_loop:
    ldb r3, [r1]
    stb r3, [r0]
    addi r0, r0, 1
    addi r1, r1, 1
    jnz r3, copy_loop
    ret

; ------------------------------------------------------------
; string_equal -- validation helper
; ------------------------------------------------------------
; input:  r0 = left, r1 = right
; output: r3 = 1 equal, 0 different
; clobbers r0-r6

string_equal:
compare_loop:
    ldb r4, [r0]
    ldb r5, [r1]
    xor r6, r4, r5
    jnz r6, strings_different
    jz r4, strings_equal
    addi r0, r0, 1
    addi r1, r1, 1
    jmp compare_loop

strings_equal:
    movi r3, 1
    ret

strings_different:
    movi r3, 0
    ret

; ------------------------------------------------------------
; Sources
; ------------------------------------------------------------

src_empty:
    .byte 0x00

src_one:
    .ascii "A"
    .byte 0x00

src_t32:
    .ascii "T32"
    .byte 0x00

src_foundry:
    .ascii "Foundry"
    .byte 0x00

src_phrase:
    .ascii "Tiny virtual machine"
    .byte 0x00

; ------------------------------------------------------------
; Guarded destinations
; ------------------------------------------------------------

; Each destination is pre-filled with 0xCC. The copy must stop after
; writing the terminating zero byte and must not disturb its guards.

dst_empty_guard_before:
    .byte 0xA1
dst_empty:
    .byte 0xCC
    .byte 0xCC
dst_empty_guard_after:
    .byte 0x1A

dst_one_guard_before:
    .byte 0xA2
dst_one:
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
dst_one_guard_after:
    .byte 0x2A

dst_t32_guard_before:
    .byte 0xA3
dst_t32:
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
dst_t32_guard_after:
    .byte 0x3A

dst_foundry_guard_before:
    .byte 0xA4
dst_foundry:
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
dst_foundry_guard_after:
    .byte 0x4A

dst_phrase_guard_before:
    .byte 0xA5
dst_phrase:
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
dst_phrase_guard_after:
    .byte 0x5A
