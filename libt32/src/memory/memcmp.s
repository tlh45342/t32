; libt32 memcmp
;
; r0 = left region
; r1 = right region
; r2 = byte count
; r3 = first unsigned-byte difference, or zero
;
; clobbers r0-r2, r4-r6
;
.section .text
.global memcmp

memcmp:
    jz   r2, memcmp_equal
memcmp_loop:
    ldb  r4, [r0]
    ldb  r5, [r1]
    xor  r6, r4, r5
    jnz  r6, memcmp_different
    addi r0, r0, 1
    addi r1, r1, 1
    subi r2, r2, 1
    jnz  r2, memcmp_loop
memcmp_equal:
    movi r3, 0
    ret
memcmp_different:
    sub  r3, r4, r5
    ret
