.org 0x1000

movi r15, 0x3000
movi r0, 10
call function
halt
function:
addi r0, r0, 32
ret
