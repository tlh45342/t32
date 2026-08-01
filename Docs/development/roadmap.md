# T32 Roadmap

## Phase 1 — Core ISA conformance

Expand validation in this order:

1. `AND`, `OR`, `XOR`, `NOT`
2. `SHL`, `SHR`, `SAR`
3. `MUL`, `MULU`
4. `DIV`, `DIVU`
5. data movement and immediate boundaries
6. memory width, endianness, alignment, and faults
7. branch direction and taken/not-taken paths
8. stack, call, trap, and interrupt behavior

## Phase 2 — Algorithm runtime

Complete:

```text
09-hex-to-string
10-string-to-hex
```

Then promote validated routines into a shared `libt32` assembly runtime.

## Phase 3 — Test infrastructure

- top-level conformance runner;
- live output and timestamped logs;
- case and directory totals;
- latest-log artifact;
- single family selection;
- machine-readable CI summary.

## Phase 4 — Formal architecture

- freeze register roles;
- freeze encoding formats;
- define every flag effect;
- define shifts and division edge behavior;
- define byte order and alignment;
- define reset, trap, exception, and interrupt models;
- assign an ISA version.

## Phase 5 — Software environment

- ABI;
- startup code;
- linker and object format;
- `libt32`;
- console primitives;
- tiny formatter;
- BIOS;
- MMIO devices.

## Phase 6 — Toolchain

- `t32-ld`;
- relocations and symbols;
- expanded `t32-cc`;
- C-to-runtime calls;
- compiled algorithm validation.

## Phase 7 — Platform and Foundry integration

- reset/boot path;
- VCONSOLE;
- timer and interrupts;
- RTC;
- VDISK;
- networking;
- `t32-node` lifecycle;
- Foundry scheduling and console attachment.

## Core completion criterion

Every instruction must have documented semantics, assembler support, runtime support, exact encoding tests, normal and boundary tests, applicable flag tests, and alias/source-preservation coverage.
