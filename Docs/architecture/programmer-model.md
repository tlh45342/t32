# T32 Programmer Model

## Scope

This document defines the machine state visible to T32 programs.

## Integer registers

T32 exposes sixteen 32-bit general-purpose registers:

```text
r0 ... r15
```

Arithmetic results are retained modulo 2^32.

## Stack pointer

**Provisional:** `r15` is the architectural stack pointer and may be written as `sp` when assembler support is added.

Current stack, call, and interrupt-return tests initialize `r15` before use.

## Program counter

The processor exposes a 32-bit program counter. The current execution environment reports it as `pc`.

The base instruction width is four bytes. A single `HALT` loaded at `0x1000` leaves the reported PC at `0x1004`.

## Condition flags

T32 currently exposes four arithmetic flags:

| Flag | Meaning |
|---|---|
| C | Carry for addition; no-borrow for subtraction and comparison |
| Z | The arithmetic result is zero |
| N | Bit 31 of the arithmetic result is set |
| V | Signed two's-complement overflow occurred |

Expanded tests verify these flags for `ADD`, `ADDI`, `SUB`, `SUBI`, `CMP`, and `CMPI`.

Subtraction uses this convention:

```text
C=1  no borrow
C=0  borrow occurred
```

Flag behavior for other instructions remains open until explicitly specified.

## Machine state

The runtime currently exposes at least:

```text
running
halted
```

`HALT` produces a halted machine with a halt reason. Faulted and stopped states are planned but are not yet fully specified here.

## Instruction counter

The runtime reports an instruction count. Tests use it to detect incorrect control flow, unexpected traps, or extra execution.

## Reset and startup

The validation environment currently performs:

```text
reset
load image.bin 0x1000
set pc 0x1000
run
```

**Open:** architectural reset values for registers, flags, memory, PC, vector base, and device state.

## Open decisions

1. Formalize `r15` as `sp`.
2. Define reset values.
3. Define flag effects for every instruction class.
4. Define trap and fault metadata.
5. Define PC, stack, and memory alignment rules.
