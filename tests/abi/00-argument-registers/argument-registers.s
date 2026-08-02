.org 0x1000
.equ STACK_TOP,0xF000
 jmp start
start:
 movi r15,STACK_TOP
 movi r7,0
 movi r0,0x11111111
 movi r1,0x22222222
 movi r2,0x33333333
 movi r3,0x44444444
 call verify
 jz r0,fail
 movi r7,1
 halt
fail:
 movi r7,0
 halt
verify:
 movi r4,0x11111111
 xor r4,r0,r4
 jnz r4,vfail
 movi r4,0x22222222
 xor r4,r1,r4
 jnz r4,vfail
 movi r4,0x33333333
 xor r4,r2,r4
 jnz r4,vfail
 movi r4,0x44444444
 xor r4,r3,r4
 jnz r4,vfail
 movi r0,1
 ret
vfail:
 movi r0,0
 ret
