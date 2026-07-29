.org 0x00008000

; First write HELLO at row 0, column 0.
start:
    movi r1, 0x90000000

    movi r0, 'H'
    stb  r0, [r1]
    addi r1, r1, 1

    movi r0, 'E'
    stb  r0, [r1]
    addi r1, r1, 1

    movi r0, 'L'
    stb  r0, [r1]
    addi r1, r1, 1

    movi r0, 'L'
    stb  r0, [r1]
    addi r1, r1, 1

    movi r0, 'O'
    stb  r0, [r1]

; Overwrite row 0, column 1.
; 0x90000000 + 1 = 0x90000001.
    movi r1, 0x90000001
    movi r0, 'A'
    stb  r0, [r1]

    halt
