; libt32 atoi -- T32 ABI 0.1
; Arguments: r0=zero-terminated unsigned decimal string
; Returns:   r0=parsed value
; Preserves: r8-r15
; Clobbers:  r1-r5
; Empty string returns zero. Validation remains intentionally minimal.
.section .text
.global atoi
atoi:
    mov  r1, r0
    movi r0, 0
atoi_loop:
    ldb  r2, [r1]
    jz   r2, atoi_done
    movi r3, '0'
    sub  r2, r2, r3
    movi r4, 10
    mul  r0, r0, r4
    add  r0, r0, r2
    addi r1, r1, 1
    jmp  atoi_loop
atoi_done:
    ret
