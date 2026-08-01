; strchr.s
;
; T32 algorithm validation: libc-style strchr.
;
; strchr contract
; ---------------
; input:
;   r0 = pointer to zero-terminated string
;   r1 = search value (only low 8 bits are used)
;
; output:
;   r2 = pointer to first matching character
;        pointer to string terminator when searching for zero
;        zero when no match is found
;
; clobbers:
;   r0, r3-r5
;
; behavior:
;   returns the first match
;   includes the terminating zero in the search domain
;   treats the search value as an unsigned byte
;
; Branch model:
;   JZ/JNZ test a named general-purpose register directly.

.org 0x00001000

.equ STACK_TOP, 0x0000F000
.equ PASS,      1
.equ FAIL,      0
.equ CASES,     7

    jmp start

string_empty:
    .byte 0

string_word:
    .byte 'F', 'o', 'u', 'n', 'd', 'r', 'y', 0

string_repeat:
    .byte 'b', 'a', 'n', 'a', 'n', 'a', 0

string_high:
    .byte 0x80, 0xFF, 0x7F, 0

start:
    movi r15, STACK_TOP
    movi r7, FAIL
    movi r14, 0

    ; Case 1: first character.
    movi r0, string_word
    movi r1, 'F'
    call strchr
    mov  r8, r2
    addi r14, r14, 1

    ; Case 2: middle character.
    movi r0, string_word
    movi r1, 'n'
    call strchr
    mov  r9, r2
    addi r14, r14, 1

    ; Case 3: duplicate returns first occurrence.
    movi r0, string_repeat
    movi r1, 'a'
    call strchr
    mov  r10, r2
    addi r14, r14, 1

    ; Case 4: absent character returns zero.
    movi r0, string_word
    movi r1, 'z'
    call strchr
    mov  r11, r2
    addi r14, r14, 1

    ; Case 5: search for terminator returns terminator address.
    movi r0, string_word
    movi r1, 0
    call strchr
    mov  r12, r2
    addi r14, r14, 1

    ; Case 6: empty string searched for terminator.
    movi r0, string_empty
    movi r1, 0
    call strchr
    mov  r13, r2
    addi r14, r14, 1

    ; Case 7: only low byte of search value is used.
    movi r0, string_high
    movi r1, 0x123456FF
    call strchr
    mov  r6, r2
    addi r14, r14, 1

    movi r5, string_high
    addi r5, r5, 1
    xor  r5, r6, r5
    jnz  r5, guest_fail

    ; Validate saved results.
    movi r6, string_word
    xor  r6, r8, r6
    jnz  r6, guest_fail

    movi r6, string_word
    addi r6, r6, 3
    xor  r6, r9, r6
    jnz  r6, guest_fail

    movi r6, string_repeat
    addi r6, r6, 1
    xor  r6, r10, r6
    jnz  r6, guest_fail

    jnz  r11, guest_fail

    movi r6, string_word
    addi r6, r6, 7
    xor  r6, r12, r6
    jnz  r6, guest_fail

    movi r6, string_empty
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
; strchr
; ------------------------------------------------------------

strchr:
    movi r3, 0xFF
    and  r1, r1, r3

strchr_loop:
    ldb  r3, [r0]

    xor  r4, r3, r1
    jz   r4, strchr_found

    ; Reaching the terminator without a match means not found.
    jz   r3, strchr_not_found

    addi r0, r0, 1
    jmp  strchr_loop

strchr_found:
    mov  r2, r0
    ret

strchr_not_found:
    movi r2, 0
    ret
