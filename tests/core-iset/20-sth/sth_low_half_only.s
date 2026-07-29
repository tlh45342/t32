.org 0x1000

movi r0, 0x3000
movi r1, 0x12345678
sth r1, [r0]
ldh r2, [r0]
halt
