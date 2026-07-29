# T32 Implementation and Validation Status

## Snapshot basis

This status is based on the supplied `tests/core-iset` tree and the reported successful `make test` run. The archive did not include assembler or runtime source, so “Assembler” and “Runtime” mean that the test assembly built and executed successfully, not that every internal implementation path was inspected.

## Summary

| Measure | Count |
|---|---:|
| Instruction test directories | 36 |
| Assembler smoke coverage | 36 / 36 |
| Runtime smoke coverage | 36 / 36 |
| Expanded conformance suites | 6 / 36 |
| Expanded cases represented | 34 |
| Known failing supplied tests | 0 |

Expanded case count:

```text
ADD   6
ADDI  6
SUB   6
SUBI  6
CMP   5
CMPI  5
Total 34
```

## Instruction matrix

Legend:

- **PASS** — demonstrated by current tests.
- **SMOKE** — one ordinary scenario only.
- **EXPANDED** — multiple boundary/flag/alias cases.
- **OPEN** — behavior still needs specification or validation.

| Opcode | Instruction | Assembler | Runtime | Test level | Encoding | Flags | Key remaining work |
|---:|---|---|---|---|---|---|---|
| 0 | HALT | PASS | PASS | SMOKE | Not exact-byte checked | N/A | Additional halt-state/reset interactions |
| 1 | NOP | PASS | PASS | SMOKE | Not exact-byte checked | Open | Register/flag preservation |
| 2 | TRAP | PASS | PASS | SMOKE | Not exact-byte checked | Open | Trap number, frame, reason, invalid vectors |
| 3 | IRET | PASS | PASS | SMOKE | Not exact-byte checked | Open | Full interrupt-frame semantics |
| 4 | CPUID | PASS | PASS | SMOKE | Not exact-byte checked | Open | ID contract and unsupported selectors |
| 8 | MOV | PASS | PASS | SMOKE | Not exact-byte checked | Open | Flags, aliases, all registers |
| 9 | MOVI | PASS | PASS | SMOKE | Not exact-byte checked | Open | Immediate boundaries and flags |
| 16 | LDB | PASS | PASS | SMOKE | Not exact-byte checked | Open | Endianness, alignment, address faults |
| 17 | LDH | PASS | PASS | SMOKE | Not exact-byte checked | Open | Endianness, alignment, address faults |
| 18 | LDW | PASS | PASS | SMOKE | Not exact-byte checked | Open | Endianness, alignment, address faults |
| 19 | STB | PASS | PASS | SMOKE | Not exact-byte checked | Open | Adjacent-byte preservation and faults |
| 20 | STH | PASS | PASS | SMOKE | Not exact-byte checked | Open | Endianness, alignment, preservation |
| 21 | STW | PASS | PASS | SMOKE | Not exact-byte checked | Open | Endianness, alignment, faults |
| 24 | ADD | PASS | PASS | EXPANDED (6) | PASS | C/Z/N/V PASS | Additional alias form if desired |
| 25 | ADDI | PASS | PASS | EXPANDED (6) | PASS | C/Z/N/V PASS | Immediate encoding boundaries |
| 26 | SUB | PASS | PASS | EXPANDED (6) | PASS | C/Z/N/V PASS | Additional alias form if desired |
| 27 | SUBI | PASS | PASS | EXPANDED (6) | PASS | C/Z/N/V PASS | Immediate encoding boundaries |
| 28 | MUL | PASS | PASS | SMOKE | Not exact-byte checked | Open | Signed cases, overflow, aliases |
| 29 | MULU | PASS | PASS | SMOKE | Not exact-byte checked | Open | High-bit operands, truncation, aliases |
| 30 | DIV | PASS | PASS | SMOKE | Not exact-byte checked | Open | Signs, rounding, zero, overflow |
| 31 | DIVU | PASS | PASS | SMOKE | Not exact-byte checked | Open | High-bit operands, zero, aliases |
| 32 | AND | PASS | PASS | SMOKE | Not exact-byte checked | Open | Masks, zero/negative, aliases |
| 33 | OR | PASS | PASS | SMOKE | Not exact-byte checked | Open | Identity/all-ones, flags, aliases |
| 34 | XOR | PASS | PASS | SMOKE | Not exact-byte checked | Open | Self-zero, masks, flags, aliases |
| 35 | NOT | PASS | PASS | SMOKE | Not exact-byte checked | Open | Zero/all-ones, flags, alias |
| 36 | SHL | PASS | PASS | SMOKE | Not exact-byte checked | Open | Counts 0/1/31/32+, carry, aliases |
| 37 | SHR | PASS | PASS | SMOKE | Not exact-byte checked | Open | Counts 0/1/31/32+, carry, aliases |
| 38 | SAR | PASS | PASS | SMOKE | Not exact-byte checked | N smoke only | Sign extension, counts, carry, aliases |
| 40 | CMP | PASS | PASS | EXPANDED (5) | PASS | C/Z/N/V PASS | Additional signed boundary pairs |
| 41 | CMPI | PASS | PASS | EXPANDED (5) | PASS | C/Z/N/V PASS | Immediate encoding boundaries |
| 42 | JMP | PASS | PASS | SMOKE | Not exact-byte checked | N/A | Backward/chain/target boundaries |
| 43 | JZ | PASS | PASS | SMOKE taken | Not exact-byte checked | N/A | Not-taken and source preservation |
| 44 | JNZ | PASS | PASS | SMOKE taken | Not exact-byte checked | N/A | Not-taken and source preservation |
| 48 | PUSH | PASS | PASS | SMOKE | Not exact-byte checked | Open | Stack direction/content/alignment |
| 49 | POP | PASS | PASS | SMOKE | Not exact-byte checked | Open | Stack content and underflow |
| 50 | CALL | PASS | PASS | SMOKE | Not exact-byte checked | Open | Return address, nesting, recursion |
| 51 | RET | PASS | PASS | SMOKE | Not exact-byte checked | Open | Underflow and malformed return address |

## Current interpretation

T32 has broad implementation and smoke coverage across its current 36-instruction core set. It does not yet have broad conformance coverage.

The correct project statement is:

```text
Core instruction set implemented: yes
Core instruction set smoke-tested: yes
Core instruction set fully validated: not yet
```

## Next validation phase

The next planned test build should expand:

```text
32 AND
33 OR
34 XOR
35 NOT
36 SHL
37 SHR
38 SAR
28 MUL
29 MULU
30 DIV
31 DIVU
```

The recommended internal order is logic, shifts, multiplication, then division. Division should be last because divide-by-zero and signed-overflow behavior may require explicit ISA decisions.
