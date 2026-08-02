; libt32 memchr
;
; r0 = memory pointer
; r1 = search value (low byte)
; r2 = byte count
; r3 = first matching address, or zero
;
; clobbers r0, r2, r4-r6
;
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
    movi r3, 0
    ret
memchr_found:
    mov  r3, r0
    ret
