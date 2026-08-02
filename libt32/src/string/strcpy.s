; libt32 strcpy
;
; r0 = destination
; r1 = source
; r2 = original destination
;
; clobbers r0, r1, r3
;
.section .text
.global strcpy

strcpy:
    mov  r2, r0
strcpy_loop:
    ldb  r3, [r1]
    stb  r3, [r0]
    addi r0, r0, 1
    addi r1, r1, 1
    jnz  r3, strcpy_loop
    ret
