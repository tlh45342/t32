; string-to-int.s
;
; T32 algorithm validation: convert a zero-terminated ASCII decimal
; string to an unsigned integer.
;
; string_to_int contract
; ----------------------
; input:
;   r0 = source string pointer
;
; output:
;   r2 = converted unsigned integer
;
; clobbers:
;   r0-r6
;
; accepted input:
;   zero or more ASCII digits followed by 0x00
;
; behavior:
;   an empty string converts to zero
;   leading zeroes are accepted
;   sign and invalid-character handling are intentionally outside
;   the scope of this first algorithm test
;
; validation results:
;   r8-r13 = six successful conversion cases
;   r14    = number of completed cases
;   r15    = restored stack pointer
;   r7     = 1 PASS, 0 FAIL

.org 0x00001000

.equ STACK_TOP, 0x0000F000
.equ PASS,      1
.equ FAIL,      0
.equ CASES,     6

start:
    movi r15, STACK_TOP
    movi r7, FAIL
    movi r14, 0

    ; Case 1: an empty string converts to zero.
    movi r0, text_empty
    call string_to_int
    movi r3, 0
    xor r4, r2, r3
    jnz r4, guest_fail
    movi r8, PASS
    addi r14, r14, 1

    ; Case 2: ASCII "0" converts to zero.
    movi r0, text_zero
    call string_to_int
    movi r3, 0
    xor r4, r2, r3
    jnz r4, guest_fail
    movi r9, PASS
    addi r14, r14, 1

    ; Case 3: a single digit converts correctly.
    movi r0, text_seven
    call string_to_int
    movi r3, 7
    xor r4, r2, r3
    jnz r4, guest_fail
    movi r10, PASS
    addi r14, r14, 1

    ; Case 4: a two-digit value converts correctly.
    movi r0, text_forty_two
    call string_to_int
    movi r3, 42
    xor r4, r2, r3
    jnz r4, guest_fail
    movi r11, PASS
    addi r14, r14, 1

    ; Case 5: a longer decimal value converts correctly.
    movi r0, text_twelve_thousand
    call string_to_int
    movi r3, 12345
    xor r4, r2, r3
    jnz r4, guest_fail
    movi r12, PASS
    addi r14, r14, 1

    ; Case 6: leading zeroes do not change the value.
    movi r0, text_leading_zeroes
    call string_to_int
    movi r3, 42
    xor r4, r2, r3
    jnz r4, guest_fail
    movi r13, PASS
    addi r14, r14, 1

    ; Validate all six cases.
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
; string_to_int
; ------------------------------------------------------------
; Convert a zero-terminated string of ASCII decimal digits.
;
; result = result * 10 + digit
;
; Multiplication by ten is intentionally expressed using ADD and a
; small loop so this test does not depend on a multiply instruction.

string_to_int:
    movi r2, 0

parse_loop:
    ldb r3, [r0]
    jz r3, parse_done

    ; Convert ASCII character to numeric digit.
    movi r4, 0x30
    sub r4, r3, r4

    ; Compute r2 = old_result * 10.
    mov r5, r2
    movi r2, 0
    movi r6, 10

multiply_by_ten:
    add r2, r2, r5
    subi r6, r6, 1
    jnz r6, multiply_by_ten

    ; Add the current digit and advance to the next character.
    add r2, r2, r4
    addi r0, r0, 1
    jmp parse_loop

parse_done:
    ret

; ------------------------------------------------------------
; Test strings
; ------------------------------------------------------------

text_empty:
    .byte 0x00

text_zero:
    .ascii "0"
    .byte 0x00

text_seven:
    .ascii "7"
    .byte 0x00

text_forty_two:
    .ascii "42"
    .byte 0x00

text_twelve_thousand:
    .ascii "12345"
    .byte 0x00

text_leading_zeroes:
    .ascii "00042"
    .byte 0x00
