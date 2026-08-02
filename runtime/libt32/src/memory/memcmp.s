; libt32 memcmp -- T32 ABI 0.1
; Arguments: r0=left, r1=right, r2=count
; Returns:   r0=first unsigned-byte difference, or zero
; Preserves: r8-r15
; Clobbers:  r1-r6
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
    movi r0, 0
    ret
memcmp_different:
    sub  r0, r4, r5
    ret
