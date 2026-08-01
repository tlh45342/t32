; int-to-string.s
;
; T32 algorithm validation: convert an unsigned integer to a
; zero-terminated ASCII decimal string.
;
; int_to_string contract
; ----------------------
; input:
;   r0 = destination buffer pointer
;   r1 = unsigned value in the range 0..99999
;
; output:
;   r2 = original destination pointer
;
; clobbers:
;   r0-r7
;
; behavior:
;   emits canonical decimal text with no leading zeroes
;   emits "0" for the value zero
;   always writes a terminating zero byte
;
; This first implementation intentionally avoids requiring DIV or MOD.
; It counts through a five-digit decimal odometer, then emits the digits.
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

; The first instruction jumps over the embedded validation data.
; This JMP uses the branch encoding with a four-byte target payload, so the
; guarded host-visible region begins at the fixed address 0x00001008.
    jmp start

; Guarded destination for the largest host-inspected case.
dst_12345_guard_before:
    .byte 0xA5
dst_12345:
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
dst_12345_guard_after:
    .byte 0x5A

; Remaining destination buffers.
dst_zero:
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC

dst_seven:
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC

dst_forty_two:
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC

dst_one_hundred:
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC

dst_one_thousand_two:
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC
    .byte 0xCC


; Expected strings

expected_zero:
    .ascii "0"
    .byte 0x00

expected_seven:
    .ascii "7"
    .byte 0x00

expected_forty_two:
    .ascii "42"
    .byte 0x00

expected_one_hundred:
    .ascii "100"
    .byte 0x00

expected_one_thousand_two:
    .ascii "1002"
    .byte 0x00

expected_12345:
    .ascii "12345"
    .byte 0x00


start:
    movi r15, STACK_TOP
    movi r7, FAIL
    movi r14, 0

    ; Case 1: zero emits "0".
    movi r0, dst_zero
    movi r1, 0
    call int_to_string
    movi r3, dst_zero
    xor r4, r2, r3
    jnz r4, guest_fail
    movi r0, dst_zero
    movi r1, expected_zero
    call string_equal
    mov r8, r3
    addi r14, r14, 1

    ; Case 2: a single digit.
    movi r0, dst_seven
    movi r1, 7
    call int_to_string
    movi r3, dst_seven
    xor r4, r2, r3
    jnz r4, guest_fail
    movi r0, dst_seven
    movi r1, expected_seven
    call string_equal
    mov r9, r3
    addi r14, r14, 1

    ; Case 3: a two-digit value.
    movi r0, dst_forty_two
    movi r1, 42
    call int_to_string
    movi r3, dst_forty_two
    xor r4, r2, r3
    jnz r4, guest_fail
    movi r0, dst_forty_two
    movi r1, expected_forty_two
    call string_equal
    mov r10, r3
    addi r14, r14, 1

    ; Case 4: transition through two decimal carries.
    movi r0, dst_one_hundred
    movi r1, 100
    call int_to_string
    movi r3, dst_one_hundred
    xor r4, r2, r3
    jnz r4, guest_fail
    movi r0, dst_one_hundred
    movi r1, expected_one_hundred
    call string_equal
    mov r11, r3
    addi r14, r14, 1

    ; Case 5: a four-digit value containing internal zeroes.
    movi r0, dst_one_thousand_two
    movi r1, 1002
    call int_to_string
    movi r3, dst_one_thousand_two
    xor r4, r2, r3
    jnz r4, guest_fail
    movi r0, dst_one_thousand_two
    movi r1, expected_one_thousand_two
    call string_equal
    mov r12, r3
    addi r14, r14, 1

    ; Case 6: a five-digit value in a guarded buffer.
    movi r0, dst_12345
    movi r1, 12345
    call int_to_string
    movi r3, dst_12345
    xor r4, r2, r3
    jnz r4, guest_fail
    movi r0, dst_12345
    movi r1, expected_12345
    call string_equal
    mov r13, r3
    addi r14, r14, 1

    ; Every conversion must have matched.
    jz r8, guest_fail
    jz r9, guest_fail
    jz r10, guest_fail
    jz r11, guest_fail
    jz r12, guest_fail
    jz r13, guest_fail

    ; Validate the guarded destination around the largest case.
    movi r0, dst_12345_guard_before
    ldb r1, [r0]
    movi r2, 0xA5
    xor r3, r1, r2
    jnz r3, guest_fail

    movi r0, dst_12345_guard_after
    ldb r1, [r0]
    movi r2, 0x5A
    xor r3, r1, r2
    jnz r3, guest_fail

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
; int_to_string
; ------------------------------------------------------------
; Maintain five decimal digits:
;   r7 r6 r5 r4 r3
;   10k 1k 100 10 1
;
; Each iteration increments the decimal odometer once. A digit is
; tested for ten by subtracting ten directly. If the result is not
; zero, ten is added back; if it is zero, the carry continues.

int_to_string:
    mov r2, r0
    movi r3, 0
    movi r4, 0
    movi r5, 0
    movi r6, 0
    movi r7, 0

    jz r1, emit_zero

count_loop:
    addi r3, r3, 1
    subi r3, r3, 10
    jz r3, carry_tens
    addi r3, r3, 10
    jmp count_complete

carry_tens:
    addi r4, r4, 1
    subi r4, r4, 10
    jz r4, carry_hundreds
    addi r4, r4, 10
    jmp count_complete

carry_hundreds:
    addi r5, r5, 1
    subi r5, r5, 10
    jz r5, carry_thousands
    addi r5, r5, 10
    jmp count_complete

carry_thousands:
    addi r6, r6, 1
    subi r6, r6, 10
    jz r6, carry_ten_thousands
    addi r6, r6, 10
    jmp count_complete

carry_ten_thousands:
    addi r7, r7, 1
    ; The documented input range stops at 99999, so no further
    ; carry is required.

count_complete:
    subi r1, r1, 1
    jnz r1, count_loop

    ; Suppress only leading zeroes. Once a position is selected,
    ; every lower position is emitted, including internal zeroes.
    jnz r7, emit_five_digits
    jnz r6, emit_four_digits
    jnz r5, emit_three_digits
    jnz r4, emit_two_digits
    jmp emit_one_digit

emit_five_digits:
    addi r7, r7, 0x30
    stb r7, [r0]
    addi r0, r0, 1

emit_four_digits:
    addi r6, r6, 0x30
    stb r6, [r0]
    addi r0, r0, 1

emit_three_digits:
    addi r5, r5, 0x30
    stb r5, [r0]
    addi r0, r0, 1

emit_two_digits:
    addi r4, r4, 0x30
    stb r4, [r0]
    addi r0, r0, 1

emit_one_digit:
    addi r3, r3, 0x30
    stb r3, [r0]
    addi r0, r0, 1
    movi r3, 0
    stb r3, [r0]
    ret

emit_zero:
    movi r3, 0x30
    stb r3, [r0]
    addi r0, r0, 1
    movi r3, 0
    stb r3, [r0]
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
    jnz r6, strings_different
    jz r4, strings_equal
    addi r0, r0, 1
    addi r1, r1, 1
    jmp string_equal_loop

strings_equal:
    movi r3, 1
    ret

strings_different:
    movi r3, 0
    ret
