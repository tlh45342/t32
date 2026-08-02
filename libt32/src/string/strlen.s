; libt32 strlen
;
; r0 = string pointer
; r1 = length
;
; clobbers r0, r2
;
.section .text
.global strlen

strlen:
    movi r1, 0
strlen_loop:
    ldb  r2, [r0]
    jz   r2, strlen_done
    addi r0, r0, 1
    addi r1, r1, 1
    jmp  strlen_loop
strlen_done:
    ret
