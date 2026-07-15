; JZ basic test

movi r0, 0
jz r0, taken
movi r1, 99
jmp done

taken:
movi r1, 42

done:
halt
