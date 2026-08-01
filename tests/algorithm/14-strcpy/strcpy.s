; strcpy.s
;
; T32 algorithm validation: libc-style strcpy.
;
; input:  r0=destination, r1=source
; output: r2=original destination
; copies through and including the zero terminator.

.org 0x00001000

.equ STACK_TOP, 0x0000F000
.equ PASS,      1
.equ FAIL,      0
.equ CASES,     5

    jmp start

source_empty:
    .byte 0
source_one:
    .byte 'A', 0
source_short:
    .byte 'T', '3', '2', 0
source_word:
    .byte 'F', 'o', 'u', 'n', 'd', 'r', 'y', 0
source_phrase:
    .byte 'T', 'i', 'n', 'y', ' ', 'v', 'i', 'r'
    .byte 't', 'u', 'a', 'l', ' ', 'm', 'a', 'c'
    .byte 'h', 'i', 'n', 'e', 0

expected_empty:
    .byte 0
expected_one:
    .byte 'A', 0
expected_short:
    .byte 'T', '3', '2', 0
expected_word:
    .byte 'F', 'o', 'u', 'n', 'd', 'r', 'y', 0
expected_phrase:
    .byte 'T', 'i', 'n', 'y', ' ', 'v', 'i', 'r'
    .byte 't', 'u', 'a', 'l', ' ', 'm', 'a', 'c'
    .byte 'h', 'i', 'n', 'e', 0

empty_guard_before:
    .byte 0xA1
empty_destination:
    .byte 0xCC, 0xCC
empty_guard_after:
    .byte 0xA2

one_guard_before:
    .byte 0xB1
one_destination:
    .byte 0xCC, 0xCC, 0xCC
one_guard_after:
    .byte 0xB2

short_guard_before:
    .byte 0xC1
short_destination:
    .byte 0xCC, 0xCC, 0xCC, 0xCC, 0xCC
short_guard_after:
    .byte 0xC2

word_guard_before:
    .byte 0xD1
word_destination:
    .byte 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC
word_guard_after:
    .byte 0xD2

phrase_guard_before:
    .byte 0xE1
phrase_destination:
    .byte 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC
    .byte 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC
    .byte 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC
phrase_guard_after:
    .byte 0xE2

start:
    movi r15, STACK_TOP
    movi r7, FAIL
    movi r14, 0

    movi r0, empty_destination
    movi r1, source_empty
    call strcpy
    mov r8, r2
    addi r14, r14, 1

    movi r0, one_destination
    movi r1, source_one
    call strcpy
    mov r9, r2
    addi r14, r14, 1

    movi r0, short_destination
    movi r1, source_short
    call strcpy
    mov r10, r2
    addi r14, r14, 1

    movi r0, word_destination
    movi r1, source_word
    call strcpy
    mov r11, r2
    addi r14, r14, 1

    movi r0, phrase_destination
    movi r1, source_phrase
    call strcpy
    mov r12, r2
    addi r14, r14, 1

    movi r6, empty_destination
    xor r6, r8, r6
    jnz r6, guest_fail
    movi r6, one_destination
    xor r6, r9, r6
    jnz r6, guest_fail
    movi r6, short_destination
    xor r6, r10, r6
    jnz r6, guest_fail
    movi r6, word_destination
    xor r6, r11, r6
    jnz r6, guest_fail
    movi r6, phrase_destination
    xor r6, r12, r6
    jnz r6, guest_fail

    movi r0, empty_destination
    movi r1, expected_empty
    call string_equal
    jnz r3, guest_fail
    movi r0, one_destination
    movi r1, expected_one
    call string_equal
    jnz r3, guest_fail
    movi r0, short_destination
    movi r1, expected_short
    call string_equal
    jnz r3, guest_fail
    movi r0, word_destination
    movi r1, expected_word
    call string_equal
    jnz r3, guest_fail
    movi r0, phrase_destination
    movi r1, expected_phrase
    call string_equal
    jnz r3, guest_fail

    movi r0, empty_guard_before
    movi r1, 0xA1
    movi r2, empty_guard_after
    movi r3, 0xA2
    call guards_match
    jnz r4, guest_fail
    movi r0, one_guard_before
    movi r1, 0xB1
    movi r2, one_guard_after
    movi r3, 0xB2
    call guards_match
    jnz r4, guest_fail
    movi r0, short_guard_before
    movi r1, 0xC1
    movi r2, short_guard_after
    movi r3, 0xC2
    call guards_match
    jnz r4, guest_fail
    movi r0, word_guard_before
    movi r1, 0xD1
    movi r2, word_guard_after
    movi r3, 0xD2
    call guards_match
    jnz r4, guest_fail
    movi r0, phrase_guard_before
    movi r1, 0xE1
    movi r2, phrase_guard_after
    movi r3, 0xE2
    call guards_match
    jnz r4, guest_fail

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

strcpy:
    mov r2, r0
strcpy_loop:
    ldb r3, [r1]
    stb r3, [r0]
    addi r0, r0, 1
    addi r1, r1, 1
    jnz r3, strcpy_loop
    ret

string_equal:
string_equal_loop:
    ldb r4, [r0]
    ldb r5, [r1]
    xor r6, r4, r5
    jnz r6, string_different
    jz r4, strings_equal
    addi r0, r0, 1
    addi r1, r1, 1
    jmp string_equal_loop
strings_equal:
    movi r3, 0
    ret
string_different:
    movi r3, 1
    ret

guards_match:
    ldb r5, [r0]
    xor r6, r5, r1
    jnz r6, guards_mismatch
    ldb r5, [r2]
    xor r6, r5, r3
    jnz r6, guards_mismatch
    movi r4, 0
    ret
guards_mismatch:
    movi r4, 1
    ret
