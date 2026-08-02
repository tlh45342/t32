; libt32 strcmp
;
; r0 = left string
; r1 = right string
; r2 = first unsigned-byte difference, or zero
;
; clobbers r0, r1, r3-r5
;
.section .text
.global strcmp

strcmp:
strcmp_loop:
    ldb  r3, [r0]
    ldb  r4, [r1]
    xor  r5, r3, r4
    jnz  r5, strcmp_different
    jz   r3, strcmp_equal
    addi r0, r0, 1
    addi r1, r1, 1
    jmp  strcmp_loop
strcmp_different:
    sub  r2, r3, r4
    ret
strcmp_equal:
    movi r2, 0
    ret
