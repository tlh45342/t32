; libt32 memset
;
; r0 = destination
; r1 = fill value (low byte used by STB)
; r2 = byte count
; r3 = original destination
;
; clobbers r0, r2
;
memset:
    mov  r3, r0
    jz   r2, memset_done
memset_loop:
    stb  r1, [r0]
    addi r0, r0, 1
    subi r2, r2, 1
    jnz  r2, memset_loop
memset_done:
    ret
