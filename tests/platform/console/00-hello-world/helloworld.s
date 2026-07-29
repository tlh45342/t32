.org 0x00008000

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
    addi r1, r1, 1

    movi r0, ' '
    stb  r0, [r1]
    addi r1, r1, 1

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