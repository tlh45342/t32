.org 0x1000
movi r0, 0x2000
movi r1, 0x1234562a
stb r1, [r0]
ldb r2, [r0]
halt
