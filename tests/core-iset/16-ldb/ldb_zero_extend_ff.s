.org 0x1000

movi r0, 0x3000
movi r1, 0xffffffff
stb r1, [r0]
ldb r2, [r0]
halt
