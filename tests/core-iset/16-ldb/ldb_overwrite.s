.org 0x1000

movi r0, 0x3000
movi r1, 0xaa
stb r1, [r0]
movi r1, 0x55
stb r1, [r0]
ldb r2, [r0]
halt
