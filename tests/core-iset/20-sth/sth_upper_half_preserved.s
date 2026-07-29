.org 0x1000

movi r0, 0x3000
movi r1, 0xaabbccdd
stw r1, [r0]
movi r1, 0x1122
sth r1, [r0]
ldw r2, [r0]
halt
