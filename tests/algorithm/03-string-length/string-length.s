; 02-string-length
;
; Validate a reusable null-terminated string-length routine with:
;   ""             -> 0
;   "A"            -> 1
;   "HELLO"        -> 5
;   "Hello World"  -> 11
;
; Register contract at HALT:
;   r7  = 0          overall PASS; nonzero means FAIL
;   r8  = 0          empty-string length
;   r9  = 1          one-character length
;   r10 = 5          HELLO length
;   r11 = 11         Hello World length
;   r12 = 4          number of completed test cases
;   r15 = stack pointer, restored to STACK_TOP
;
; strlen routine:
;   input:  r0 = address of null-terminated string
;   output: r1 = length, excluding terminator
;   clobbers r0, r2

.org 0x00001000

.equ STACK_TOP, 0x0000F000

start:
    movi r15, STACK_TOP
    movi r7, 1
    movi r12, 0

    movi r0, string_empty
    call strlen
    mov  r8, r1
    movi r3, 0
    xor  r4, r1, r3
    jnz  r4, fail
    addi r12, r12, 1

    movi r0, string_one
    call strlen
    mov  r9, r1
    movi r3, 1
    xor  r4, r1, r3
    jnz  r4, fail
    addi r12, r12, 1

    movi r0, string_hello
    call strlen
    mov  r10, r1
    movi r3, 5
    xor  r4, r1, r3
    jnz  r4, fail
    addi r12, r12, 1

    movi r0, string_hello_world
    call strlen
    mov  r11, r1
    movi r3, 11
    xor  r4, r1, r3
    jnz  r4, fail
    addi r12, r12, 1

pass:
    movi r7, 0
    halt

fail:
    halt

strlen:
    movi r1, 0

strlen_loop:
    ldb  r2, [r0]
    jz   r2, strlen_done
    addi r0, r0, 1
    addi r1, r1, 1
    jmp  strlen_loop

strlen_done:
    ret

string_empty:
    .byte 0

string_one:
    .ascii "A"
    .byte 0

string_hello:
    .ascii "HELLO"
    .byte 0

string_hello_world:
    .ascii "Hello World"
    .byte 0
