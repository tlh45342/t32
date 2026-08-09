; libt32 I/O ABI 0.1
;
; putchar
;   input:  r0 = character byte
;   output: r0 = character byte
;
; puts
;   input:  r0 = pointer to zero-terminated byte string
;   output: r0 = 0
.extern putchar
.extern puts
