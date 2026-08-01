; strstr.s
;
; T32 algorithm validation: libc-style strstr.
;
; strstr contract
; ---------------
; input:
;   r0 = pointer to haystack string
;   r1 = pointer to needle string
;
; output:
;   r2 = pointer to first occurrence of needle in haystack
;        haystack pointer when needle is empty
;        zero when no match is found
;
; clobbers:
;   r0-r6
;
; behavior:
;   returns first match
;   supports repeated-prefix candidates
;   does not modify either input string
;
; Branch model:
;   JZ/JNZ test a named general-purpose register directly.

.org 0x00001000

.equ STACK_TOP, 0x0000F000
.equ PASS,      1
.equ FAIL,      0
.equ CASES,     8

    jmp start

hay_empty:
    .byte 0

needle_empty:
    .byte 0

hay_foundry:
    .byte 'F', 'o', 'u', 'n', 'd', 'r', 'y', 0

needle_foundry:
    .byte 'F', 'o', 'u', 'n', 'd', 'r', 'y', 0

needle_found:
    .byte 'F', 'o', 'u', 'n', 'd', 0

needle_dry:
    .byte 'd', 'r', 'y', 0

needle_missing:
    .byte 'x', 'y', 'z', 0

hay_repeat:
    .byte 'a', 'b', 'a', 'b', 'a', 'b', 'a', 'c', 0

needle_repeat:
    .byte 'a', 'b', 'a', 'c', 0

hay_banana:
    .byte 'b', 'a', 'n', 'a', 'n', 'a', 0

needle_ana:
    .byte 'a', 'n', 'a', 0

start:
    movi r15, STACK_TOP
    movi r7, FAIL
    movi r14, 0

    ; Case 1: empty needle returns haystack.
    movi r0, hay_foundry
    movi r1, needle_empty
    call strstr
    mov  r8, r2
    addi r14, r14, 1

    ; Case 2: empty haystack and empty needle returns haystack.
    movi r0, hay_empty
    movi r1, needle_empty
    call strstr
    mov  r9, r2
    addi r14, r14, 1

    ; Case 3: full-string match.
    movi r0, hay_foundry
    movi r1, needle_foundry
    call strstr
    mov  r10, r2
    addi r14, r14, 1

    ; Case 4: prefix match.
    movi r0, hay_foundry
    movi r1, needle_found
    call strstr
    mov  r11, r2
    addi r14, r14, 1

    ; Case 5: suffix match.
    movi r0, hay_foundry
    movi r1, needle_dry
    call strstr
    mov  r12, r2
    addi r14, r14, 1

    ; Case 6: absent needle.
    movi r0, hay_foundry
    movi r1, needle_missing
    call strstr
    mov  r13, r2
    addi r14, r14, 1

    ; Case 7: repeated prefix requires advancing candidates.
    movi r0, hay_repeat
    movi r1, needle_repeat
    call strstr
    mov  r6, r2
    addi r14, r14, 1

    movi r5, hay_repeat
    addi r5, r5, 4
    xor  r5, r6, r5
    jnz  r5, guest_fail

    ; Case 8: duplicate substring returns first occurrence.
    movi r0, hay_banana
    movi r1, needle_ana
    call strstr
    mov  r6, r2
    addi r14, r14, 1

    movi r5, hay_banana
    addi r5, r5, 1
    xor  r5, r6, r5
    jnz  r5, guest_fail

    ; Validate saved results.
    movi r6, hay_foundry
    xor  r6, r8, r6
    jnz  r6, guest_fail

    movi r6, hay_empty
    xor  r6, r9, r6
    jnz  r6, guest_fail

    movi r6, hay_foundry
    xor  r6, r10, r6
    jnz  r6, guest_fail

    movi r6, hay_foundry
    xor  r6, r11, r6
    jnz  r6, guest_fail

    movi r6, hay_foundry
    addi r6, r6, 4
    xor  r6, r12, r6
    jnz  r6, guest_fail

    jnz  r13, guest_fail

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
; strstr
; ------------------------------------------------------------

strstr:
    ; Empty needle matches at haystack start.
    ldb  r3, [r1]
    jz   r3, strstr_empty_needle

    ; r4 = current haystack candidate
    ; r5 = original needle pointer
    mov  r4, r0
    mov  r5, r1

strstr_candidate:
    ldb  r3, [r4]
    jz   r3, strstr_not_found

    mov  r0, r4
    mov  r1, r5

strstr_compare:
    ; Reaching the needle terminator means every needle byte matched.
    ldb  r3, [r1]
    jz   r3, strstr_found

    ; Haystack ended before the needle.
    ldb  r6, [r0]
    jz   r6, strstr_advance

    xor  r6, r3, r6
    jnz  r6, strstr_advance

    addi r0, r0, 1
    addi r1, r1, 1
    jmp  strstr_compare

strstr_advance:
    addi r4, r4, 1
    jmp  strstr_candidate

strstr_found:
    mov  r2, r4
    ret

strstr_empty_needle:
    mov  r2, r0
    ret

strstr_not_found:
    movi r2, 0
    ret
