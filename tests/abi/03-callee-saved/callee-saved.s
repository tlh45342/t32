.org 0x1000
.equ STACK_TOP,0xF000
 jmp start
start:
 movi r15,STACK_TOP
 movi r7,0
 movi r8,0x08080808
 movi r9,0x09090909
 movi r10,0x10101010
 movi r11,0x11111111
 movi r12,0x12121212
 movi r13,0x13131313
 movi r14,0x14141414
 call preserve
 movi r0,0x08080808
 xor r0,r8,r0
 jnz r0,fail
 movi r0,0x09090909
 xor r0,r9,r0
 jnz r0,fail
 movi r0,0x10101010
 xor r0,r10,r0
 jnz r0,fail
 movi r0,0x11111111
 xor r0,r11,r0
 jnz r0,fail
 movi r0,0x12121212
 xor r0,r12,r0
 jnz r0,fail
 movi r0,0x13131313
 xor r0,r13,r0
 jnz r0,fail
 movi r0,0x14141414
 xor r0,r14,r0
 jnz r0,fail
 movi r7,1
 halt
fail:
 movi r7,0
 halt
preserve:
 push r8
 push r9
 push r10
 push r11
 push r12
 push r13
 push r14
 movi r8,8
 movi r9,9
 movi r10,10
 movi r11,11
 movi r12,12
 movi r13,13
 movi r14,14
 pop r14
 pop r13
 pop r12
 pop r11
 pop r10
 pop r9
 pop r8
 ret
