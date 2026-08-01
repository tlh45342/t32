; string-reverse.s
;
; T32 algorithm validation: reverse a zero-terminated string in place.
;
; string_reverse contract
; -----------------------
; input:
;   r0 = pointer to a zero-terminated string
;
; output:
;   r0 = original string pointer
;
; clobbers:
;   r1-r6
;
; validation results:
;   r8-r13 = six successful reverse cases
;   r14    = number of completed cases
;   r15    = restored stack pointer
;   r7     = 1 PASS, 0 FAIL

.org 0x00001000

.equ STACK_TOP, 0x0000F000
.equ PASS,      1
.equ FAIL,      0
.equ CASES,     6

; Keep one guarded test string at a stable, host-visible address.
; T32 JMP occupies eight bytes, so host_guard_before begins at 0x00001008.
    jmp start

host_guard_before:
    .byte 0xA5
host_string:
    .ascii "Foundry"
    .byte 0x00
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
host_guard_after:
    .byte 0x5A

start:
    movi r15, STACK_TOP
    movi r7, FAIL
    movi r14, 0

    ; Case 1: empty string remains empty.
    movi r0, case_empty
    call string_reverse
    movi r1, expected_empty
    call string_equal
    mov r8, r3
    addi r14, r14, 1

    ; Case 2: one-character string remains unchanged.
    movi r0, case_one
    call string_reverse
    movi r1, expected_one
    call string_equal
    mov r9, r3
    addi r14, r14, 1

    ; Case 3: even-length string swaps exactly once.
    movi r0, case_even
    call string_reverse
    movi r1, expected_even
    call string_equal
    mov r10, r3
    addi r14, r14, 1

    ; Case 4: odd-length string preserves its center byte.
    movi r0, case_t32
    call string_reverse
    movi r1, expected_t32
    call string_equal
    mov r11, r3
    addi r14, r14, 1

    ; Case 5: normal word in the host-visible guarded buffer.
    movi r0, host_string
    call string_reverse
    movi r1, expected_foundry
    call string_equal
    mov r12, r3
    addi r14, r14, 1

    ; Case 6: phrase containing a space.
    movi r0, case_phrase
    call string_reverse
    movi r1, expected_phrase
    call string_equal
    mov r13, r3
    addi r14, r14, 1

    ; Every case must pass.
    jz r8, guest_fail
    jz r9, guest_fail
    jz r10, guest_fail
    jz r11, guest_fail
    jz r12, guest_fail
    jz r13, guest_fail

    ; Verify the host-visible guards were not modified.
    movi r0, host_guard_before
    ldb r1, [r0]
    movi r2, 0xA5
    xor r3, r1, r2
    jnz r3, guest_fail

    movi r0, host_guard_after
    ldb r1, [r0]
    movi r2, 0x5A
    xor r3, r1, r2
    jnz r3, guest_fail

    ; Verify completion count and stack restoration.
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
; string_reverse
; ------------------------------------------------------------
; Reverse a zero-terminated string in place.
;
; input:  r0 = string pointer
; output: r0 = original string pointer
; clobbers r1-r6

string_reverse:
    mov r6, r0              ; preserve original pointer
    mov r1, r0              ; scan pointer
    movi r2, 0              ; length

reverse_length_loop:
    ldb r3, [r1]
    jz r3, reverse_length_done
    addi r1, r1, 1
    addi r2, r2, 1
    jmp reverse_length_loop

reverse_length_done:
    ; Empty and one-character strings require no swaps.
    jz r2, reverse_done
    movi r3, 1
    xor r4, r2, r3
    jz r4, reverse_done

    ; r0 = left pointer, r1 = final nonzero byte, r2 = bytes remaining.
    mov r0, r6
    mov r1, r6
    add r1, r1, r2
    subi r1, r1, 1

reverse_swap_loop:
    ; Stop with zero bytes remaining (even length).
    jz r2, reverse_done

    ; Stop with one byte remaining (odd-length center).
    movi r3, 1
    xor r4, r2, r3
    jz r4, reverse_done

    ldb r4, [r0]
    ldb r5, [r1]
    stb r5, [r0]
    stb r4, [r1]

    addi r0, r0, 1
    subi r1, r1, 1
    subi r2, r2, 2
    jmp reverse_swap_loop

reverse_done:
    mov r0, r6
    ret

; ------------------------------------------------------------
; string_equal -- validation helper
; ------------------------------------------------------------
; input:  r0 = left, r1 = right
; output: r3 = 1 equal, 0 different
; clobbers r0-r6

string_equal:
string_equal_loop:
    ldb r4, [r0]
    ldb r5, [r1]
    xor r6, r4, r5
    jnz r6, string_not_equal
    jz r4, string_is_equal
    addi r0, r0, 1
    addi r1, r1, 1
    jmp string_equal_loop

string_is_equal:
    movi r3, 1
    ret

string_not_equal:
    movi r3, 0
    ret

; ------------------------------------------------------------
; Mutable test strings
; ------------------------------------------------------------

case_empty:
    .byte 0x00

case_one:
    .ascii "A"
    .byte 0x00

case_even:
    .ascii "AB"
    .byte 0x00

case_t32:
    .ascii "T32"
    .byte 0x00

case_phrase:
    .ascii "hello world"
    .byte 0x00

; ------------------------------------------------------------
; Expected strings
; ------------------------------------------------------------

expected_empty:
    .byte 0x00

expected_one:
    .ascii "A"
    .byte 0x00

expected_even:
    .ascii "BA"
    .byte 0x00

expected_t32:
    .ascii "23T"
    .byte 0x00

expected_foundry:
    .ascii "yrdnuoF"
    .byte 0x00

expected_phrase:
    .ascii "dlrow olleh"
    .byte 0x00
