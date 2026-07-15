; STB basic test
; Expected:
;   memory[0x2000] = 0x2A

movi r0, 0x2000
movi r1, 42
stb r1, [r0]
halt
