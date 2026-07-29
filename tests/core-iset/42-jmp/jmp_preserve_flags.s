.org 0x1000
movi r0, 0xffffffff
movi r1, 1
add r2, r0, r1
jmp target
movi r3, 99
target:
halt
