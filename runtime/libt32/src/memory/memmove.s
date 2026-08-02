; libt32 memmove -- T32 ABI 0.1
; Arguments: r0=destination, r1=source, r2=count
; Returns:   r0=original destination
; Preserves: r8-r15
; Clobbers:  r1-r6
.section .text
.global memmove
memmove:
    mov  r3, r0
    jz   r2, memmove_done
    xor  r4, r0, r1
    jz   r4, memmove_done
    mov  r4, r1
    addi r4, r4, 1
    mov  r5, r2
    subi r5, r5, 1
    jz   r5, memmove_forward
memmove_overlap_scan:
    xor  r6, r0, r4
    jz   r6, memmove_backward
    addi r4, r4, 1
    subi r5, r5, 1
    jnz  r5, memmove_overlap_scan
memmove_forward:
    ldb  r4, [r1]
    stb  r4, [r0]
    addi r0, r0, 1
    addi r1, r1, 1
    subi r2, r2, 1
    jnz  r2, memmove_forward
    jmp  memmove_done
memmove_backward:
    add  r0, r0, r2
    subi r0, r0, 1
    add  r1, r1, r2
    subi r1, r1, 1
memmove_backward_loop:
    ldb  r4, [r1]
    stb  r4, [r0]
    subi r0, r0, 1
    subi r1, r1, 1
    subi r2, r2, 1
    jnz  r2, memmove_backward_loop
memmove_done:
    mov  r0, r3
    ret
