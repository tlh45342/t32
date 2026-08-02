.org 0x1000
.equ STACK_TOP,0xF000
 jmp start
start:
 movi r15,STACK_TOP
 movi r7,0
 call outer
 jz r0,fail
 movi r7,1
 halt
fail:
 movi r7,0
 halt
outer:
 movi r4,3
 and r5,r15,r4
 jnz r5,bad
 push r8
 movi r4,3
 and r5,r15,r4
 jnz r5,restore_bad
 call inner
 jz r0,restore_bad
 pop r8
 movi r0,1
 ret
restore_bad:
 pop r8
bad:
 movi r0,0
 ret
inner:
 movi r4,3
 and r5,r15,r4
 jnz r5,inner_bad
 movi r0,1
 ret
inner_bad:
 movi r0,0
 ret
