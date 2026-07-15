; JNZ basic test

movi r0, 1
jnz r0, taken
movi r1, 99
jmp done

taken:
movi r1, 42

done:
halt
