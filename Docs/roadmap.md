# T32 Roadmap

## Current position

The present core instruction inventory has assembler support, runtime execution, and smoke tests across 36 instructions. Six arithmetic/compare instructions have expanded conformance tests.

## Phase 1 — Core ISA conformance

### Completed

- Core instruction directory structure
- Assembler/runtime smoke tests for the current 36-instruction set
- Expanded validation for ADD, ADDI, SUB, SUBI, CMP, and CMPI
- C/Z/N/V verification for expanded arithmetic and comparison cases
- Exact encoding checks in expanded arithmetic/compare suites

### Next: logic, shifts, multiplication, and division

Expand in this order:

1. AND, OR, XOR, NOT
2. SHL, SHR, SAR
3. MUL, MULU
4. DIV, DIVU

Goals:

- exact encodings;
- ordinary and boundary results;
- source preservation;
- destination aliasing;
- all applicable flags;
- explicit exceptional behavior.

### Following: remaining core groups

- MOV and MOVI boundaries/flags
- memory width, byte order, alignment, and faults
- taken/not-taken and backward control flow
- stack contents, nesting, underflow, and interrupt frames
- system-instruction fault and identity behavior

## Phase 2 — Test infrastructure

- Top-level Python conformance runner
- Live console output plus timestamped logs
- Final test/case totals
- `latest` log artifact
- Run one instruction or one family
- Machine-readable summary for CI
- Optional generated implementation-status data

## Phase 3 — Formal ISA specification

- Freeze register roles and aliases
- Freeze instruction encoding formats
- Define all flag effects
- Define shift and division edge behavior
- Define endianness and alignment
- Define trap/interrupt model
- Assign an ISA version number

## Phase 4 — Toolchain

- Object-file format
- `t32-ld` linker
- relocations and symbols
- runtime startup code
- assembler expressions/macros as needed
- `t32-cc` compiler path

## Phase 5 — Machine environment

- Boot/reset model
- Timer and interrupts
- memory-mapped text console
- RTC and basic devices
- disk/storage interface
- small operational validation programs

## Phase 6 — Foundry integration

- `t32-node` release and debug builds
- VM registration and lifecycle
- console attachment
- image upload/storage
- scheduling through Foundry

## Completion criterion for the core ISA

The core ISA is complete only when every instruction has:

```text
documented semantics
assembler implementation
runtime implementation
exact encoding test
normal-result test
boundary/exception tests
complete applicable flag tests
alias/source-preservation tests
```
