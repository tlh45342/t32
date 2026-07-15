#T32

tests/core-iset/

 0  HALT
 1  NOP
 2  TRAP
 3  IRET
 4  CPUID

 8  MOV
 9  MOVI

 16 LDB
 17 LDH
 18 LDW
 19 STB
 20 STH
 21 STW

 24 ADD
 25 ADDI
 26 SUB
 27 SUBI
 28 MUL
 29 MULU
 30 DIV
 31 DIVU

 32 AND
 33 OR
 34 XOR
 35 NOT
 36 SHL
 37 SHR
 38 SAR

 40 CMP
 41 CMPI
 42 JMP
 43 JZ
 44 JNZ

 48 PUSH
 49 POP
 50 CALL
 51 RET



0–4      System
8–9      Data movement
16–21    Memory
24–31    Arithmetic
32–38    Logic and shifts
40–44    Compare and branches
48–51    Stack and calls



# T32 Instruction Set

## HALT

Syntax

    halt

Description

Stops execution.

Flags

None.

Encoding

Opcode 0x00

Example

    halt

---------------------------------------

## MOVI

Syntax

    movi rN, immediate

Description

Loads a 32-bit immediate.

Flags

Z
N

Encoding

Opcode 0x02

Example

    movi r0,47
