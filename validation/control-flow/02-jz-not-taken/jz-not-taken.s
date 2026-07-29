.org 0x1000

movi r0, 1
jz r0, taken
movi r1, 42
jmp done
taken:
movi r1, 99
done:
halt
