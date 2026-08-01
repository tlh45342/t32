# T32 Instruction Set

## Inventory

| Opcode | Mnemonic | Class | Syntax |
|---:|---|---|---|
| 0 | HALT | System | `halt` |
| 1 | NOP | System | `nop` |
| 2 | TRAP | System | `trap imm` |
| 3 | IRET | System | `iret` |
| 4 | CPUID | System | `cpuid rd` |
| 8 | MOV | Data movement | `mov rd, ra` |
| 9 | MOVI | Data movement | `movi rd, imm32` |
| 16 | LDB | Memory | `ldb rd, [ra]` |
| 17 | LDH | Memory | `ldh rd, [ra]` |
| 18 | LDW | Memory | `ldw rd, [ra]` |
| 19 | STB | Memory | `stb rs, [ra]` |
| 20 | STH | Memory | `sth rs, [ra]` |
| 21 | STW | Memory | `stw rs, [ra]` |
| 24 | ADD | Arithmetic | `add rd, ra, rb` |
| 25 | ADDI | Arithmetic | `addi rd, ra, imm` |
| 26 | SUB | Arithmetic | `sub rd, ra, rb` |
| 27 | SUBI | Arithmetic | `subi rd, ra, imm` |
| 28 | MUL | Arithmetic | `mul rd, ra, rb` |
| 29 | MULU | Arithmetic | `mulu rd, ra, rb` |
| 30 | DIV | Arithmetic | `div rd, ra, rb` |
| 31 | DIVU | Arithmetic | `divu rd, ra, rb` |
| 32 | AND | Logic | `and rd, ra, rb` |
| 33 | OR | Logic | `or rd, ra, rb` |
| 34 | XOR | Logic | `xor rd, ra, rb` |
| 35 | NOT | Logic | `not rd, ra` |
| 36 | SHL | Shift | `shl rd, ra, rb` |
| 37 | SHR | Shift | `shr rd, ra, rb` |
| 38 | SAR | Shift | `sar rd, ra, rb` |
| 40 | CMP | Compare | `cmp ra, rb` |
| 41 | CMPI | Compare | `cmpi ra, imm` |
| 42 | JMP | Control flow | `jmp target` |
| 43 | JZ | Control flow | `jz ra, target` |
| 44 | JNZ | Control flow | `jnz ra, target` |
| 48 | PUSH | Stack | `push ra` |
| 49 | POP | Stack | `pop rd` |
| 50 | CALL | Call | `call target` |
| 51 | RET | Call | `ret` |

## Opcode groups

```text
0-4      system
8-9      data movement
16-21    memory
24-31    arithmetic
32-38    logic and shifts
40-44    compare and control flow
48-51    stack and calls
```

## Verified behavior

- `MOV` copies a register value.
- `MOVI` loads a 32-bit immediate.
- Byte, halfword, and word loads/stores round-trip correctly in current tests.
- `ADD`, `ADDI`, `SUB`, `SUBI`, `CMP`, and `CMPI` have expanded arithmetic and flag validation.
- `JZ` and `JNZ` inspect a named register directly.
- `PUSH`, `POP`, `CALL`, `RET`, and `IRET` have smoke coverage with stack restoration checks.

## Open behavior

The following still require formal decisions and expanded conformance tests:

- logical and shift flag effects;
- multiplication overflow and signedness details;
- division rounding and exceptional behavior;
- memory endianness, alignment, and faults;
- precise stack frame and interrupt frame layout;
- trap vectoring and exception metadata;
- reset behavior.
