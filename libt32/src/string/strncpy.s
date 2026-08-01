; libt32 strncpy
;
; r0 = destination
; r1 = source
; r2 = byte count
; r3 = original destination
;
; clobbers r0-r5
;
strncpy:
    mov  r3, r0
    jz   r2, strncpy_done
strncpy_copy:
    ldb  r4, [r1]
    stb  r4, [r0]
    addi r0, r0, 1
    addi r1, r1, 1
    subi r2, r2, 1
    jz   r2, strncpy_done
    jnz  r4, strncpy_copy
    movi r5, 0
strncpy_pad:
    stb  r5, [r0]
    addi r0, r0, 1
    subi r2, r2, 1
    jnz  r2, strncpy_pad
strncpy_done:
    ret
