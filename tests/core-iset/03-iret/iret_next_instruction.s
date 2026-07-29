.org 0x1000
movi r15, 0x3000
movi r0, target
push r0
iret
target:
movi r1, 42
halt
