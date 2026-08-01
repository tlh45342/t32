; libt32 strchr
;
; r0 = string
; r1 = search value (low byte)
; r2 = first matching address, terminator address for zero, or zero
;
; clobbers r0, r1, r3-r4
;
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
strchr_found:
    mov  r2, r0
    ret
strchr_not_found:
    movi r2, 0
    ret
