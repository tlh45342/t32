.org 0x1000
movi r0, 2
loop:
subi r0, r0, 1
jz r0, done
jmp loop
done:
movi r1, 42
halt
