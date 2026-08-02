; libt32 memcpy -- T32 ABI 0.1
; Arguments: r0=destination, r1=source, r2=count
; Returns:   r0=original destination
; Preserves: r8-r15
; Clobbers:  r1-r4
; Source and destination must not overlap.
.section .text
.global memcpy
memcpy:
    mov  r3, r0
    jz   r2, memcpy_done
memcpy_loop:
    ldb  r4, [r1]
    stb  r4, [r0]
    addi r0, r0, 1
    addi r1, r1, 1
    subi r2, r2, 1
    jnz  r2, memcpy_loop
memcpy_done:
    mov  r0, r3
    ret
