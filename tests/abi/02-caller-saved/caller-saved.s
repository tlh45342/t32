.org 0x1000
.equ STACK_TOP,0xF000
 jmp start
start:
 movi r15,STACK_TOP
 movi r7,0
 movi r4,0x44444444
 movi r5,0x55555555
 movi r6,0x66666666
 push r4
 push r5
 push r6
 call clobber
 pop r6
 pop r5
 pop r4
 movi r0,0x44444444
 xor r0,r4,r0
 jnz r0,fail
 movi r0,0x55555555
 xor r0,r5,r0
 jnz r0,fail
 movi r0,0x66666666
 xor r0,r6,r0
 jnz r0,fail
 movi r7,1
 halt
fail:
 movi r7,0
 halt
clobber:
 movi r0,0xA0000000
 movi r1,0xA1111111
 movi r2,0xA2222222
 movi r3,0xA3333333
 movi r4,0xA4444444
 movi r5,0xA5555555
 movi r6,0xA6666666
 movi r7,0xA7777777
 ret
