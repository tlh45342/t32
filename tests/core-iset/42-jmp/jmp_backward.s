.org 0x1000
movi r0, 0
movi r1, 3
loop:
addi r0, r0, 1
subi r1, r1, 1
jz r1, done
jmp loop
done:
halt
