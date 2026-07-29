.org 0x00008000

start:
    movi r0, message
    movi r1, 0x90000000

copy_loop:
    ldb  r2, [r0]
    jz   r2, done

    stb  r2, [r1]

    addi r0, r0, 1
    addi r1, r1, 1
    jmp  copy_loop

done:
    halt

message:
    .byte 'H'
    .byte 'E'
    .byte 'L'
    .byte 'L'
    .byte 'O'
    .byte ' '
    .byte 'W'
    .byte 'O'
    .byte 'R'
    .byte 'L'
    .byte 'D'
    .byte 0