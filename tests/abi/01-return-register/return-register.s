.org 0x1000
.equ STACK_TOP,0xF000
 jmp start
start:
 movi r15,STACK_TOP
 movi r7,0
 call value
 movi r1,0x13579BDF
 xor r1,r0,r1
 jnz r1,fail
 movi r7,1
 halt
fail:
 movi r7,0
 halt
value:
 movi r0,0x13579BDF
 ret
