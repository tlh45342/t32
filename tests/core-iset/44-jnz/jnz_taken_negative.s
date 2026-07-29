.org 0x1000
movi r0, 0x80000000
jnz r0, taken
movi r1, 99
taken:
movi r1, 42
halt
