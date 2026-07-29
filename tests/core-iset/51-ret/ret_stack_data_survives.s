.org 0x1000
movi r15, 0x3000
movi r4, 0x12345678
push r4
call function
pop r5
halt
function:
ret
