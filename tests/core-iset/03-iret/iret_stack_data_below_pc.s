.org 0x1000
movi r15, 0x3000
movi r4, 0x12345678
push r4
movi r0, target
push r0
iret
halt
target:
pop r5
halt
