; SUB instruction test
;
; Expected:
;   r0 = 42
;   r1 = 20
;   r2 = 22

movi r0, 42
movi r1, 20
sub  r2, r0, r1
halt