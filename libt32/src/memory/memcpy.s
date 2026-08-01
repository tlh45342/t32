; libt32 memcpy
;
; r0 = destination
; r1 = source
; r2 = byte count
; r3 = original destination
;
; Source and destination must not overlap.
; clobbers r0-r2, r4
;
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
    ret
