.org 0x1000

movi r0, 0x3000
movi r1, 0x44332211
stw r1, [r0]
ldb r2, [r0]
movi r0, 0x3001
ldb r3, [r0]
movi r0, 0x3002
ldb r4, [r0]
movi r0, 0x3003
ldb r5, [r0]
halt
