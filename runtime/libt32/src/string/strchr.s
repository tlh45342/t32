; libt32 strchr -- T32 ABI 0.1
; Arguments: r0=string, r1=search byte
; Returns:   r0=first match, terminator for zero, or zero
; Preserves: r8-r15
; Clobbers:  r1-r4
.section .text
.global strchr
strchr:
    movi r3, 0xFF
    and  r1, r1, r3
strchr_loop:
    ldb  r3, [r0]
    xor  r4, r3, r1
    jz   r4, strchr_found
    jz   r3, strchr_not_found
    addi r0, r0, 1
    jmp  strchr_loop
strchr_not_found:
    movi r0, 0
strchr_found:
    ret
