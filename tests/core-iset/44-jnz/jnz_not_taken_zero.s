.org 0x1000
movi r0, 0
jnz r0, taken
movi r1, 42
halt
taken:
movi r1, 99
halt
