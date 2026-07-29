.org 0x1000
movi r15, 0x3000
movi r0, 0
call increment
call increment
halt
increment:
addi r0, r0, 1
ret
