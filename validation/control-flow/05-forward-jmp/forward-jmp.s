.org 0x1000

movi r0, 10
jmp target
movi r0, 99
target:
addi r0, r0, 32
halt
