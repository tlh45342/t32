.org 0x00008000

; 01-memory-copy
;
; Copy 16 bytes from 0x00009000 to 0x00009100.
; Verify the copied destination byte-for-byte.
;
; Result:
;   r7 = 0  PASS
;   r7 = 1  FAIL

start:
    movi r7, 0

; ----------------------------------------------------------------------
; Initialize the source buffer.
; ----------------------------------------------------------------------

    movi r1, 0x00009000

    movi r3, 0x00
    stb  r3, [r1]
    addi r1, r1, 1

    movi r3, 0x11
    stb  r3, [r1]
    addi r1, r1, 1

    movi r3, 0x22
    stb  r3, [r1]
    addi r1, r1, 1

    movi r3, 0x33
    stb  r3, [r1]
    addi r1, r1, 1

    movi r3, 0x44
    stb  r3, [r1]
    addi r1, r1, 1

    movi r3, 0x55
    stb  r3, [r1]
    addi r1, r1, 1

    movi r3, 0x66
    stb  r3, [r1]
    addi r1, r1, 1

    movi r3, 0x77
    stb  r3, [r1]
    addi r1, r1, 1

    movi r3, 0x88
    stb  r3, [r1]
    addi r1, r1, 1

    movi r3, 0x99
    stb  r3, [r1]
    addi r1, r1, 1

    movi r3, 0xAA
    stb  r3, [r1]
    addi r1, r1, 1

    movi r3, 0xBB
    stb  r3, [r1]
    addi r1, r1, 1

    movi r3, 0xCC
    stb  r3, [r1]
    addi r1, r1, 1

    movi r3, 0xDD
    stb  r3, [r1]
    addi r1, r1, 1

    movi r3, 0xEE
    stb  r3, [r1]
    addi r1, r1, 1

    movi r3, 0xFF
    stb  r3, [r1]

; ----------------------------------------------------------------------
; Clear the destination buffer first.
;
; This prevents a false success caused by destination memory already
; containing the expected values.
; ----------------------------------------------------------------------

    movi r1, 0x00009100
    movi r2, 16
    movi r3, 0

clear_destination:
    stb  r3, [r1]
    addi r1, r1, 1
    subi r2, r2, 1
    jnz  r2, clear_destination

; ----------------------------------------------------------------------
; Copy source to destination.
; ----------------------------------------------------------------------

    movi r1, 0x00009000     ; source pointer
    movi r2, 0x00009100     ; destination pointer
    movi r3, 16             ; byte count

copy_loop:
    ldb  r4, [r1]
    stb  r4, [r2]

    addi r1, r1, 1
    addi r2, r2, 1
    subi r3, r3, 1
    jnz  r3, copy_loop

; ----------------------------------------------------------------------
; Verify destination against source.
; ----------------------------------------------------------------------

    movi r1, 0x00009000
    movi r2, 0x00009100
    movi r3, 16

verify_loop:
    ldb  r4, [r1]
    ldb  r5, [r2]

    xor  r6, r4, r5
    jnz  r6, failed

    addi r1, r1, 1
    addi r2, r2, 1
    subi r3, r3, 1
    jnz  r3, verify_loop

passed:
    movi r7, 0
    halt

failed:
    movi r7, 1
    halt