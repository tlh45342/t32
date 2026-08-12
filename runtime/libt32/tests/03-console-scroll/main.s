.global _start
.extern puts

.section .text
_start:
    movi r15, 0x0000F000

    movi r0, line01
    call puts
    movi r0, line02
    call puts
    movi r0, line03
    call puts
    movi r0, line04
    call puts
    movi r0, line05
    call puts
    movi r0, line06
    call puts
    movi r0, line07
    call puts
    movi r0, line08
    call puts
    movi r0, line09
    call puts
    movi r0, line10
    call puts
    movi r0, line11
    call puts
    movi r0, line12
    call puts

    movi r0, 42
    halt

.section .data
line01: .ascii "SCROLL LINE 01" .byte 0
line02: .ascii "SCROLL LINE 02" .byte 0
line03: .ascii "SCROLL LINE 03" .byte 0
line04: .ascii "SCROLL LINE 04" .byte 0
line05: .ascii "SCROLL LINE 05" .byte 0
line06: .ascii "SCROLL LINE 06" .byte 0
line07: .ascii "SCROLL LINE 07" .byte 0
line08: .ascii "SCROLL LINE 08" .byte 0
line09: .ascii "SCROLL LINE 09" .byte 0
line10: .ascii "SCROLL LINE 10" .byte 0
line11: .ascii "SCROLL LINE 11" .byte 0
line12: .ascii "SCROLL LINE 12" .byte 0
