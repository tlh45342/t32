.org 0x1000
movi r0, 1
jmp target
movi r1, 0xdeadbeef
movi r2, 0xcafebabe
target:
movi r3, 42
halt
