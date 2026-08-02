.org 0x1000
.equ STACK_TOP,0xF000
 jmp start
start:
 movi r15,STACK_TOP
 movi r7,0
 movi r8,0x88888888
 movi r0,5
 call one
 movi r1,13
 xor r1,r0,r1
 jnz r1,fail
 movi r1,0x88888888
 xor r1,r8,r1
 jnz r1,fail
 movi r7,1
 halt
fail:
 movi r7,0
 halt
one:
 push r8
 mov r8,r0
 call two
 add r0,r0,r8
 pop r8
 ret
two:
 addi r0,r0,3
 ret
