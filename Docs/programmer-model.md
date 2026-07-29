# T32 Programmer Model

## Scope

This document describes the machine state visible to T32 programs. Statements marked **Verified** are directly supported by the supplied tests. Other details remain provisional until the runtime source and formal ISA definition are reviewed together.

## Integer registers

T32 exposes 16 general-purpose 32-bit registers:

```text
r0 ... r15
```

**Verified:** tests use `r0` through `r15`, and register values are reported as eight hexadecimal digits.

### Stack pointer convention

The current tests use `r15` as the stack pointer for `PUSH`, `POP`, `CALL`, `RET`, and `IRET` scenarios.

```asm
movi r15, 0x3000
```

**Provisional:** `r15` should be formally designated as `sp` in the ISA specification if this is intended to be architectural rather than a runtime convention.

## Program counter

The processor has a 32-bit program counter, displayed by the runtime as `pc`.

The test environment loads programs at `0x00001000` and sets:

```text
pc = 0x00001000
```

**Verified:** a one-instruction `HALT` advances the reported PC to `0x00001004`, which is evidence that the base instruction width is four bytes.

## Condition flags

The current runtime reports four arithmetic condition flags:

| Name | Meaning |
|---|---|
| C | Carry for addition; no-borrow for subtraction/comparison |
| Z | Result is zero |
| N | Result bit 31 is set |
| V | Signed overflow |

Expanded ADD, ADDI, SUB, SUBI, CMP, and CMPI tests verify the following model:

- Addition sets `C` on unsigned carry out.
- Subtraction and comparison set `C` when no borrow is required.
- `Z` reflects a zero 32-bit result.
- `N` reflects bit 31 of the 32-bit result.
- `V` reflects signed two's-complement overflow.

**Open:** flag behavior for MOV/MOVI, logical operations, shifts, multiplication, division, and memory operations must be defined and validated explicitly.

## Machine states

The runtime exposes at least these observable state concepts:

- running
- halted

Tests expect:

```text
state=halted
```

The HALT test additionally expects:

```text
reason=HALT instruction
```

Earlier planning notes mention stopped and faulted states, but those states are not proven by the supplied test set.

## Instruction counter

The runtime reports an instruction counter:

```text
instructions=N
```

Every test checks the expected instruction count. This is part of the test contract and helps detect incorrect branches, unexpected traps, and extra execution.

## Reset and startup convention

The test harness performs this sequence:

```text
reset
load <program>.bin 0x1000
set pc 0x1000
set run steps <N>
run
```

**Provisional:** reset values for general registers, flags, PC, and memory are not yet formally specified by the supplied documentation.

## Arithmetic width

All verified arithmetic behavior is modulo 2^32. Register results are retained as 32-bit values.

Examples proven by expanded tests:

```text
0xffffffff + 1 = 0x00000000, C=1, Z=1
0 - 1          = 0xffffffff, C=0, N=1
0x7fffffff + 1 = 0x80000000, V=1, N=1
0x80000000 - 1 = 0x7fffffff, V=1
```

## Open programmer-model decisions

The following need explicit architectural decisions:

1. Whether `r15` is permanently reserved as the stack pointer.
2. Whether symbolic aliases such as `sp` and `pc` are assembler-visible.
3. Reset values for all registers and flags.
4. Whether all instructions update flags or only selected classes.
5. Trap/fault state and exception metadata.
6. Alignment requirements for PC, stack, and memory accesses.
