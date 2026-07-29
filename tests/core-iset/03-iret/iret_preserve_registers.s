.org 0x1000
movi r15, 0x3000
movi r7, 0xabcdef01
movi r0, target
push r0
iret
halt
target:
halt
