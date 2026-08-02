.org 0x1000
.equ STACK_TOP,0xF000
 jmp start
start:
 movi r15,STACK_TOP
 movi r7,0
 movi r0,1
 movi r1,2
 movi r2,3
 movi r3,4
 movi r4,6
 push r4
 movi r4,5
 push r4
 call sum6
 addi r15,r15,8
 movi r1,21
 xor r1,r0,r1
 jnz r1,fail
 movi r1,STACK_TOP
 xor r1,r15,r1
 jnz r1,fail
 movi r7,1
 halt
fail:
 movi r7,0
 halt
sum6:
 add r0,r0,r1
 add r0,r0,r2
 add r0,r0,r3
 mov r4,r15
 addi r4,r4,4
 ldw r5,[r4]
 add r0,r0,r5
 addi r4,r4,4
 ldw r5,[r4]
 add r0,r0,r5
 ret
