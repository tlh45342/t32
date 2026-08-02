; libt32 atoi
;
; Initial unsigned-decimal form.
; r0 = pointer to zero-terminated decimal digits
; r1 = parsed value
;
; Empty string returns zero. Input validation is intentionally minimal.
; clobbers r0, r2-r5
;
.section .text
.global atoi

atoi:
    movi r1, 0
atoi_loop:
    ldb  r2, [r0]
    jz   r2, atoi_done
    movi r3, '0'
    sub  r2, r2, r3
    movi r4, 10
    mul  r1, r1, r4
    add  r1, r1, r2
    addi r0, r0, 1
    jmp  atoi_loop
atoi_done:
    ret
