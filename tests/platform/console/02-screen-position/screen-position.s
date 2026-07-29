.org 0x00008000

; Write HELLO at row 0, column 0.
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

; Write WORLD at row 2, column 10.
; Offset = (2 * 80) + 10 = 170 = 0xAA.
    movi r1, 0x900000AA

    movi r0, 'W'
    stb  r0, [r1]
    addi r1, r1, 1

    movi r0, 'O'
    stb  r0, [r1]
    addi r1, r1, 1

    movi r0, 'R'
    stb  r0, [r1]
    addi r1, r1, 1

    movi r0, 'L'
    stb  r0, [r1]
    addi r1, r1, 1

    movi r0, 'D'
    stb  r0, [r1]

    halt
