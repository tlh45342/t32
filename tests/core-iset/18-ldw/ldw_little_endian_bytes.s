.org 0x1000

movi r0, 0x3000
movi r1, 0x11
stb r1, [r0]
movi r0, 0x3001
movi r1, 0x22
stb r1, [r0]
movi r0, 0x3002
movi r1, 0x33
stb r1, [r0]
movi r0, 0x3003
movi r1, 0x44
stb r1, [r0]
movi r0, 0x3000
ldw r2, [r0]
halt
