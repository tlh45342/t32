.org 0x1000

movi r0, 0x3000
movi r1, 0xdeadbeef
movi r3, 0
cmpi r3, 0
stw r1, [r0]
halt
