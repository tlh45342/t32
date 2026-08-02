; libt32 strrev -- T32 ABI 0.1 (non-standard)
; Arguments: r0=mutable string
; Returns:   r0=original string
; Preserves: r8-r15
; Clobbers:  r1-r6
.section .text
.global strrev
strrev:
    mov  r1, r0
    mov  r2, r0
strrev_find_end:
    ldb  r3, [r2]
    jz   r3, strrev_end_found
    addi r2, r2, 1
    jmp  strrev_find_end
strrev_end_found:
    xor  r4, r1, r2
    jz   r4, strrev_done
    subi r2, r2, 1
strrev_loop:
    xor  r4, r1, r2
    jz   r4, strrev_done
    ldb  r5, [r1]
    ldb  r6, [r2]
    stb  r6, [r1]
    stb  r5, [r2]
    addi r1, r1, 1
    subi r2, r2, 1
    xor  r4, r1, r2
    jnz  r4, strrev_loop
strrev_done:
    ret
