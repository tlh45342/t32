.org 0x1000
.equ STACK_TOP,0xF000
 jmp start
start:
 movi r15,STACK_TOP
 movi r7,0
 call use_stack
 movi r0,STACK_TOP
 xor r0,r15,r0
 jnz r0,fail
 movi r7,1
 halt
fail:
 movi r7,0
 halt
use_stack:
 push r8
 push r9
 subi r15,r15,16
 movi r8,0xAAAAAAAA
 movi r9,0xBBBBBBBB
 addi r15,r15,16
 pop r9
 pop r8
 ret
