# T32 Instruction Set

## Instruction inventory

The current test tree contains the following 36 instructions.

| Opcode | Mnemonic | Class | Current syntax demonstrated by tests |
|---:|---|---|---|
| 0 | HALT | System | `halt` |
| 1 | NOP | System | `nop` |
| 2 | TRAP | System | `trap 7` |
| 3 | IRET | System | `iret` |
| 4 | CPUID | System | `cpuid r0` |
| 8 | MOV | Data movement | `mov r1, r0` |
| 9 | MOVI | Data movement | `movi r0, 42` |
| 16 | LDB | Memory | `ldb r2, [r0]` |
| 17 | LDH | Memory | `ldh r2, [r0]` |
| 18 | LDW | Memory | `ldw r2, [r0]` |
| 19 | STB | Memory | `stb r1, [r0]` |
| 20 | STH | Memory | `sth r1, [r0]` |
| 21 | STW | Memory | `stw r1, [r0]` |
| 24 | ADD | Arithmetic | `add r2, r0, r1` |
| 25 | ADDI | Arithmetic | `addi r1, r0, 22` |
| 26 | SUB | Arithmetic | `sub r2, r0, r1` |
| 27 | SUBI | Arithmetic | `subi r1, r0, 20` |
| 28 | MUL | Arithmetic | `mul r2, r0, r1` |
| 29 | MULU | Arithmetic | `mulu r2, r0, r1` |
| 30 | DIV | Arithmetic | `div r2, r0, r1` |
| 31 | DIVU | Arithmetic | `divu r2, r0, r1` |
| 32 | AND | Logic | `and r2, r0, r1` |
| 33 | OR | Logic | `or r2, r0, r1` |
| 34 | XOR | Logic | `xor r2, r0, r1` |
| 35 | NOT | Logic | `not r1, r0` |
| 36 | SHL | Shift | `shl r2, r0, r1` |
| 37 | SHR | Shift | `shr r2, r0, r1` |
| 38 | SAR | Shift | `sar r2, r0, r1` |
| 40 | CMP | Compare | `cmp r0, r1` |
| 41 | CMPI | Compare | `cmpi r0, 42` |
| 42 | JMP | Control flow | `jmp target` |
| 43 | JZ | Control flow | `jz r0, taken` |
| 44 | JNZ | Control flow | `jnz r0, taken` |
| 48 | PUSH | Stack | `push r0` |
| 49 | POP | Stack | `pop r1` |
| 50 | CALL | Call | `call function` |
| 51 | RET | Call | `ret` |

The numeric opcode values are derived from the existing test-directory numbering and expanded encoding checks. They should be cross-checked against the assembler/runtime source before being declared immutable.

## Opcode ranges

```text
0-4      System
8-9      Data movement
16-21    Memory
24-31    Arithmetic
32-38    Logic and shifts
40-44    Compare and control flow
48-51    Stack and calls
```

Unused values leave room for future additions while preserving class grouping.

## Verified instruction semantics

### HALT

Stops execution. The test expects the machine to enter `state=halted` with `reason=HALT instruction`.

### NOP

Executes without changing the tested program state and proceeds to the following instruction.

### MOV and MOVI

- `MOV` copies a register value to another register.
- `MOVI` loads a 32-bit immediate value.

### Loads and stores

- `LDB` and `STB` operate on the low 8 bits.
- `LDH` and `STH` operate on the low 16 bits.
- `LDW` and `STW` operate on 32 bits.

The current smoke tests use register-indirect addressing such as `[r0]`.

**Open:** sign extension, alignment behavior, endianness, addressing offsets, and access faults require formal documentation and expanded tests.

### ADD and ADDI

Compute a 32-bit sum and update `C`, `Z`, `N`, and `V`. Expanded tests cover normal, zero, carry, signed overflow, negative result, zero immediate, and destination/source aliasing.

### SUB and SUBI

Compute a 32-bit difference and update `C`, `Z`, `N`, and `V`. `C=1` means no borrow. Expanded tests cover normal, equal operands, borrow, underflow, signed overflow, zero immediate, and aliasing.

### CMP and CMPI

Perform subtraction for flag generation without storing the arithmetic result. Expanded tests verify equality, greater-than, less-than, negative operands, and signed-overflow cases while preserving source registers.

### MUL, MULU, DIV, and DIVU

Smoke tests verify ordinary positive integer results.

**Open:** signed interpretation, overflow/truncation, division rounding, divide-by-zero, `INT32_MIN / -1`, flag effects, and aliasing require expanded validation.

### AND, OR, XOR, and NOT

Smoke tests verify one ordinary result for each operation.

**Open:** zero/negative flags, carry/overflow preservation or clearing, identity cases, masks, source preservation, and aliasing require expanded validation.

### SHL, SHR, and SAR

Smoke tests verify one ordinary one-bit shift. The SAR smoke test additionally verifies a negative result.

**Open:** shift by zero, shift by 31, shift counts of 32 or greater, carry-out behavior, sign fill, flag effects, and aliasing require expanded validation.

### JMP, JZ, and JNZ

- `JMP` performs an unconditional label branch.
- `JZ rN, label` branches when the tested register is zero.
- `JNZ rN, label` branches when the tested register is nonzero.

This syntax indicates that JZ/JNZ currently test a register directly rather than consuming the processor Z flag.

### PUSH, POP, CALL, RET, and IRET

Smoke tests demonstrate stack operations using `r15` initialized to `0x3000`. CALL/RET and IRET tests verify return behavior and stack restoration.

**Open:** precise stack growth direction, stored return address, interrupt frame format, stack alignment, and failure behavior require formal specification.

## Flag summary

| Instruction group | Current verified flag coverage |
|---|---|
| ADD, ADDI | C, Z, N, V |
| SUB, SUBI | C, Z, N, V |
| CMP, CMPI | C, Z, N, V |
| SAR | Negative result checked in smoke test |
| All others | Not comprehensively specified or validated |
