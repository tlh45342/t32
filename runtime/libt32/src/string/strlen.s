; libt32 strlen -- T32 ABI 0.1
; Arguments: r0=string
; Returns:   r0=length
; Preserves: r8-r15
; Clobbers:  r1-r2
.section .text
.global strlen
strlen:
    mov  r1, r0
    movi r0, 0
strlen_loop:
    ldb  r2, [r1]
    jz   r2, strlen_done
    addi r1, r1, 1
    addi r0, r0, 1
    jmp  strlen_loop
strlen_done:
    ret
