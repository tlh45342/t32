.org 0x1000

movi r0, 0x3001
movi r1, 0xaabbccdd
stw r1, [r0]
movi r0, 0x3002
ldb r2, [r0]
halt
