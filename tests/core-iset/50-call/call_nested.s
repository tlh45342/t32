.org 0x1000
movi r15, 0x3000
call outer
halt
outer:
movi r0, 1
call inner
addi r0, r0, 1
ret
inner:
addi r0, r0, 40
ret
