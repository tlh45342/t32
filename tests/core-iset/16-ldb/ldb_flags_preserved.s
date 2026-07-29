.org 0x1000

movi r0, 0x3000
movi r1, 0x5a
stb r1, [r0]
movi r3, 0
cmpi r3, 0
ldb r2, [r0]
halt
