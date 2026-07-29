.org 0x1000

movi r15, 0x3000
movi r0, 10
call function_a
halt
function_a:
addi r0, r0, 20
call function_b
addi r0, r0, 5
ret
function_b:
addi r0, r0, 7
ret
