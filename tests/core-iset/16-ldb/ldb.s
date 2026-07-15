; LDB basic test
; Harness should initialize memory[0x2000] = 0x2A.
; Expected:
;   r1 = 0x0000002A

movi r0, 0x2000
ldb r1, [r0]
halt
