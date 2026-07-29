.org 0x1000
movi r15, 0x3000
movi r0, target
push r0
iret
movi r1, 0xdeadbeef
movi r2, 0xcafebabe
target:
movi r3, 42
halt
