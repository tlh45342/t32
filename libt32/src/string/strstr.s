; libt32 strstr
;
; r0 = haystack
; r1 = needle
; r2 = first occurrence, haystack for empty needle, or zero
;
; clobbers r0-r6
;
.section .text
.global strstr

strstr:
    ldb  r3, [r1]
    jz   r3, strstr_empty_needle
    mov  r4, r0
    mov  r5, r1
strstr_candidate:
    ldb  r3, [r4]
    jz   r3, strstr_not_found
    mov  r0, r4
    mov  r1, r5
strstr_compare:
    ldb  r3, [r1]
    jz   r3, strstr_found
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
