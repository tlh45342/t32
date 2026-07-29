.org 0x1000
movi r0, 0
jz r0, taken
movi r1, 0xdeadbeef
movi r2, 0xcafebabe
taken:
movi r3, 42
halt
