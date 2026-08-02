; libt32 memset -- T32 ABI 0.1
;
; Arguments: r0=destination, r1=fill byte, r2=count
; Returns:   r0=original destination
; Preserves: r8-r15
; Clobbers:  r1-r4
.section .text
.global memset
memset:
    mov  r3, r0
    jz   r2, memset_done
memset_loop:
    stb  r1, [r0]
    addi r0, r0, 1
    subi r2, r2, 1
    jnz  r2, memset_loop
memset_done:
    mov  r0, r3
    ret
