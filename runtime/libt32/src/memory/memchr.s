; libt32 memchr -- T32 ABI 0.1
; Arguments: r0=memory, r1=search byte, r2=count
; Returns:   r0=first matching address, or zero
; Preserves: r8-r15
; Clobbers:  r1-r6
.section .text
.global memchr
memchr:
    jz   r2, memchr_not_found
    movi r5, 0xFF
    and  r6, r1, r5
memchr_loop:
    ldb  r4, [r0]
    xor  r5, r4, r6
    jz   r5, memchr_found
    addi r0, r0, 1
    subi r2, r2, 1
    jnz  r2, memchr_loop
memchr_not_found:
    movi r0, 0
memchr_found:
    ret
