.org 0x1000
movi r0, 0x2000
movi r1, 0x12342a2b
sth r1, [r0]
ldh r2, [r0]
halt
