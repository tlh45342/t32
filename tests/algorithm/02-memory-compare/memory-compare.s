; memory-compare.s
;
; T32 algorithm validation: fixed-length byte comparison.
;
; mem_compare contract
; --------------------
; input:
;   r0 = pointer to left memory block
;   r1 = pointer to right memory block
;   r2 = byte count
;
; output:
;   r3 = 0 when equal
;   r3 = 1 when different
;
; clobbers:
;   r0-r6
;
; validation results:
;   r8  = equal blocks
;   r9  = difference at first byte
;   r10 = difference in middle
;   r11 = difference at last byte
;   r12 = zero-length comparison
;   r14 = number of completed cases
;   r15 = restored stack pointer
;   r7  = 1 PASS, 0 FAIL
;
; Branch model
; ------------
; T32 JZ/JNZ test a named general-purpose register directly:
;
;   jz  ra, target
;   jnz ra, target
;
; CMP/CMPI flags are not consumed by JZ/JNZ in this program.
; Equality checks therefore produce an explicit zero/nonzero register value.

.org 0x00001000

.equ STACK_TOP, 0x0000F000
.equ PASS,      1
.equ FAIL,      0
.equ CASES,     5

start:
    movi r15, STACK_TOP
    movi r7, FAIL
    movi r14, 0

    ; ------------------------------------------------------------
    ; Case 1: identical 16-byte blocks compare equal.
    ; ------------------------------------------------------------

    movi r0, block_equal_a
    movi r1, block_equal_b
    movi r2, 16
    call mem_compare
    mov  r8, r3
    addi r14, r14, 1

    ; ------------------------------------------------------------
    ; Case 2: difference at byte zero is detected.
    ; ------------------------------------------------------------

    movi r0, block_first_a
    movi r1, block_first_b
    movi r2, 16
    call mem_compare
    mov  r9, r3
    addi r14, r14, 1

    ; ------------------------------------------------------------
    ; Case 3: difference at byte seven is detected.
    ; ------------------------------------------------------------

    movi r0, block_middle_a
    movi r1, block_middle_b
    movi r2, 16
    call mem_compare
    mov  r10, r3
    addi r14, r14, 1

    ; ------------------------------------------------------------
    ; Case 4: difference at the final byte is detected.
    ; ------------------------------------------------------------

    movi r0, block_last_a
    movi r1, block_last_b
    movi r2, 16
    call mem_compare
    mov  r11, r3
    addi r14, r14, 1

    ; ------------------------------------------------------------
    ; Case 5: zero bytes always compare equal.
    ;
    ; Deliberately use different blocks to prove that the routine
    ; returns before reading memory when count is zero.
    ; ------------------------------------------------------------

    movi r0, block_first_a
    movi r1, block_first_b
    movi r2, 0
    call mem_compare
    mov  r12, r3
    addi r14, r14, 1

    ; ------------------------------------------------------------
    ; Guest-side validation
    ;
    ; Each comparison below computes:
    ;
    ;   actual XOR expected
    ;
    ; Zero means the expected value matched. JNZ branches on the
    ; explicit result register when validation fails.
    ; ------------------------------------------------------------

    movi r6, 0
    xor  r6, r8, r6
    jnz  r6, guest_fail

    movi r6, 1
    xor  r6, r9, r6
    jnz  r6, guest_fail

    movi r6, 1
    xor  r6, r10, r6
    jnz  r6, guest_fail

    movi r6, 1
    xor  r6, r11, r6
    jnz  r6, guest_fail

    movi r6, 0
    xor  r6, r12, r6
    jnz  r6, guest_fail

    movi r6, CASES
    xor  r6, r14, r6
    jnz  r6, guest_fail

    movi r6, STACK_TOP
    xor  r6, r15, r6
    jnz  r6, guest_fail

guest_pass:
    movi r7, PASS
    halt

guest_fail:
    movi r7, FAIL
    halt

; ------------------------------------------------------------
; mem_compare
;
; Compare exactly r2 bytes.
;
; The routine returns early at the first differing byte.
; A count of zero is equal and does not access either memory block.
; ------------------------------------------------------------

mem_compare:
    jz   r2, mem_equal

mem_compare_loop:
    ldb  r4, [r0]
    ldb  r5, [r1]

    ; XOR is zero only when the two bytes are equal.
    xor  r6, r4, r5
    jnz  r6, mem_different

    addi r0, r0, 1
    addi r1, r1, 1
    subi r2, r2, 1
    jnz  r2, mem_compare_loop

mem_equal:
    movi r3, 0
    ret

mem_different:
    movi r3, 1
    ret

; ------------------------------------------------------------
; Test data
; ------------------------------------------------------------

block_equal_a:
    .byte 0x00
    .byte 0x11
    .byte 0x22
    .byte 0x33
    .byte 0x44
    .byte 0x55
    .byte 0x66
    .byte 0x77
    .byte 0x88
    .byte 0x99
    .byte 0xAA
    .byte 0xBB
    .byte 0xCC
    .byte 0xDD
    .byte 0xEE
    .byte 0xFF

block_equal_b:
    .byte 0x00
    .byte 0x11
    .byte 0x22
    .byte 0x33
    .byte 0x44
    .byte 0x55
    .byte 0x66
    .byte 0x77
    .byte 0x88
    .byte 0x99
    .byte 0xAA
    .byte 0xBB
    .byte 0xCC
    .byte 0xDD
    .byte 0xEE
    .byte 0xFF

block_first_a:
    .byte 0x10
    .byte 0x21
    .byte 0x32
    .byte 0x43
    .byte 0x54
    .byte 0x65
    .byte 0x76
    .byte 0x87
    .byte 0x98
    .byte 0xA9
    .byte 0xBA
    .byte 0xCB
    .byte 0xDC
    .byte 0xED
    .byte 0xFE
    .byte 0x0F

block_first_b:
    .byte 0x11
    .byte 0x21
    .byte 0x32
    .byte 0x43
    .byte 0x54
    .byte 0x65
    .byte 0x76
    .byte 0x87
    .byte 0x98
    .byte 0xA9
    .byte 0xBA
    .byte 0xCB
    .byte 0xDC
    .byte 0xED
    .byte 0xFE
    .byte 0x0F

block_middle_a:
    .byte 0x01
    .byte 0x02
    .byte 0x03
    .byte 0x04
    .byte 0x05
    .byte 0x06
    .byte 0x07
    .byte 0x08
    .byte 0x09
    .byte 0x0A
    .byte 0x0B
    .byte 0x0C
    .byte 0x0D
    .byte 0x0E
    .byte 0x0F
    .byte 0x10

block_middle_b:
    .byte 0x01
    .byte 0x02
    .byte 0x03
    .byte 0x04
    .byte 0x05
    .byte 0x06
    .byte 0x07
    .byte 0xFF
    .byte 0x09
    .byte 0x0A
    .byte 0x0B
    .byte 0x0C
    .byte 0x0D
    .byte 0x0E
    .byte 0x0F
    .byte 0x10

block_last_a:
    .byte 0xF0
    .byte 0xE1
    .byte 0xD2
    .byte 0xC3
    .byte 0xB4
    .byte 0xA5
    .byte 0x96
    .byte 0x87
    .byte 0x78
    .byte 0x69
    .byte 0x5A
    .byte 0x4B
    .byte 0x3C
    .byte 0x2D
    .byte 0x1E
    .byte 0x0F

block_last_b:
    .byte 0xF0
    .byte 0xE1
    .byte 0xD2
    .byte 0xC3
    .byte 0xB4
    .byte 0xA5
    .byte 0x96
    .byte 0x87
    .byte 0x78
    .byte 0x69
    .byte 0x5A
    .byte 0x4B
    .byte 0x3C
    .byte 0x2D
    .byte 0x1E
    .byte 0x10
