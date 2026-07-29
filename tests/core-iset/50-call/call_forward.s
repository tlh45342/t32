.org 0x1000
movi r15, 0x3000
call function
halt
function:
movi r0, 42
ret
