; CALL basic test

movi r15, 0x3000
call function
halt

function:
movi r0, 42
ret
