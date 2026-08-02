; libt32 strcmp -- T32 ABI 0.1
; Arguments: r0=left, r1=right
; Returns:   r0=first unsigned-byte difference, or zero
; Preserves: r8-r15
; Clobbers:  r1-r5
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
    sub  r0, r3, r4
    ret
strcmp_equal:
    movi r0, 0
    ret
