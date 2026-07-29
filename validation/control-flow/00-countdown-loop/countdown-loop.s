.org 0x1000

movi r0, 5
movi r1, 0

loop:
addi r1, r1, 1
subi r0, r0, 1
jnz r0, loop

halt
