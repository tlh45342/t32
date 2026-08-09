; libt32 puts -- T32 ABI 0.1
; Arguments: r0=pointer to zero-terminated byte string
; Returns:   r0=0
; Preserves: r8-r15
; Clobbers:  r0-r7
;
; Writes the string through putchar and appends one newline.
.section .text
.global puts
.extern putchar

puts:
    push r8
    mov  r8, r0

puts_loop:
    ldb  r0, [r8]
    jz   r0, puts_newline
    call putchar
    addi r8, r8, 1
    jmp  puts_loop

puts_newline:
    movi r0, 10
    call putchar
    movi r0, 0
    pop  r8
    ret
