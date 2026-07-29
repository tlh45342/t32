.org 0x1000

movi r0, 0x3000
movi r1, 0x1234
sth r1, [r0]
ldb r2, [r0]
movi r0, 0x3001
ldb r3, [r0]
halt
