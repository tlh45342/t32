; 00-memory-fill
;
; Fill 16 bytes at 0x00009000 with 0xA5, then verify them.
;
; This flat binary is assembled for the same address used by test.script.
;
; Register contract at HALT:
;   r0 = 0x00009000   buffer base
;   r1 = 0x00009010   fill pointer after 16 bytes
;   r2 = 0x000000A5   fill value
;   r3 = 0x00000000   fill count exhausted
;   r4 = 0x00009010   verification pointer after 16 bytes
;   r5 = 0x00000000   verification count exhausted
;   r6 = 0x00000000   final comparison result
;   r7 = 0x00000000   PASS, nonzero = FAIL

.org 0x00001000

.equ BUFFER_BASE, 0x00009000
.equ BUFFER_SIZE, 16
.equ FILL_VALUE,  0xA5

start:
    movi r0, BUFFER_BASE
    mov  r1, r0
    movi r2, FILL_VALUE
    movi r3, BUFFER_SIZE
    movi r7, 1

fill_loop:
    stb  r2, [r1]
    addi r1, r1, 1
    subi r3, r3, 1
    jnz  r3, fill_loop

    mov  r4, r0
    movi r5, BUFFER_SIZE

verify_loop:
    ldb  r6, [r4]
    xor  r6, r6, r2
    jnz  r6, fail

    addi r4, r4, 1
    subi r5, r5, 1
    jnz  r5, verify_loop

pass:
    movi r7, 0
    halt

fail:
    movi r7, 1
    halt
