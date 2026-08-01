; libt32 strrev
;
; Non-standard helper.
; r0 = mutable zero-terminated string
; r1 = original string pointer
;
; clobbers r0-r6
;
strrev:
    mov  r1, r0
    mov  r2, r0
strrev_find_end:
    ldb  r3, [r2]
    jz   r3, strrev_end_found
    addi r2, r2, 1
    jmp  strrev_find_end
strrev_end_found:
    xor  r4, r0, r2
    jz   r4, strrev_done
    subi r2, r2, 1
strrev_loop:
    xor  r4, r0, r2
    jz   r4, strrev_done
    ldb  r5, [r0]
    ldb  r6, [r2]
    stb  r6, [r0]
    stb  r5, [r2]
    addi r0, r0, 1
    subi r2, r2, 1
    xor  r4, r0, r2
    jnz  r4, strrev_loop
strrev_done:
    ret
