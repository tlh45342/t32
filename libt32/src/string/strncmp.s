; libt32 strncmp
;
; r0 = left string
; r1 = right string
; r2 = maximum byte count
; r3 = first unsigned-byte difference, or zero
;
; clobbers r0-r2, r4-r6
;
.section .text
.global strncmp

strncmp:
    jz   r2, strncmp_equal
strncmp_loop:
    ldb  r4, [r0]
    ldb  r5, [r1]
    xor  r6, r4, r5
    jnz  r6, strncmp_different
    jz   r4, strncmp_equal
    addi r0, r0, 1
    addi r1, r1, 1
    subi r2, r2, 1
    jnz  r2, strncmp_loop
strncmp_equal:
    movi r3, 0
    ret
strncmp_different:
    sub  r3, r4, r5
    ret
