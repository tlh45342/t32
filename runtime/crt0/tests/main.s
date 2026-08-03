;
; crt0 integration test program
;
; int main(void)
; {
;     return 5;
; }

.section .text
.global main

main:
    movi r0, 5
    ret
