; libt32 strcpy -- T32 ABI 0.1
; Arguments: r0=destination, r1=source
; Returns:   r0=original destination
; Preserves: r8-r15
; Clobbers:  r1-r3
.section .text
.global strcpy
strcpy:
    mov  r2, r0
strcpy_loop:
    ldb  r3, [r1]
    stb  r3, [r0]
    addi r0, r0, 1
    addi r1, r1, 1
    jnz  r3, strcpy_loop
    mov  r0, r2
    ret
