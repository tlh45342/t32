# T32 Implementation and Validation Status

## Core ISA

The current 36-instruction core set has assembler support, runtime execution, and smoke tests.

Expanded arithmetic and comparison coverage exists for:

```text
ADD
ADDI
SUB
SUBI
CMP
CMPI
```

The accurate project statement is:

```text
Core instruction set implemented: yes
Core instruction set smoke-tested: yes
Core instruction set fully validated: not yet
```

## Algorithm suite

Validated algorithm directories currently include:

```text
00-memory-fill
01-memory-copy
02-memory-compare
03-string-length
04-string-compare
05-string-copy
06-string-to-int
07-int-to-string
08-string-reverse
```

Planned next algorithms:

```text
09-hex-to-string
10-string-to-hex
```

## Highest-priority conformance gaps

1. Logic: `AND`, `OR`, `XOR`, `NOT`
2. Shifts: `SHL`, `SHR`, `SAR`
3. Multiplication: `MUL`, `MULU`
4. Division: `DIV`, `DIVU`
5. Memory byte order, alignment, and faults
6. Stack and interrupt frame behavior
7. Reset, trap, and exception semantics

## Status maintenance

This file should be updated whenever:

- a new instruction or algorithm suite is added;
- an open architectural behavior is decided;
- a smoke test becomes expanded conformance coverage;
- a discrepancy between documentation and implementation is discovered.
