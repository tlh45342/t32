# T32 Roadmap

## Current execution roadmap — August 11, 2026

The platform now has a validated BIOS -> BOOT.BIN -> compiler-built Stage3
monitor chain, keyboard/display, block storage, Bootinfo, BIOS disk service, and
RTC.

### Sprint A — external application execution
- add Stage3 `run`;
- load a flat binary at a documented application address;
- prove HELLO.BIN;
- return cleanly to Stage3;
- build externally from `G:\X2`.

### Sprint B — first workload
- build deterministic `SORT.BIN`;
- verify sorted output;
- keep it independently buildable;
- later promote a clean version to `samples/`.

### Sprint C — application services
- define a small versioned SVC ABI;
- provide `libt32` wrappers;
- start with console, exit, time/ticks, and disk read.

### Sprint D — timing
- keep RTC for UTC wall clock;
- add a monotonic timer/counter;
- benchmark SORT reproducibly.

### Sprint E — executable/filesystem evolution
- introduce a tiny versioned executable header when justified;
- add hello/sort/memory/clock/disk programs;
- evolve beyond T32D toward a real filesystem, with ext2 as a target.

### Parallel toolchain work
- continue `t32-cc`;
- add target `stdio.h`, `stdlib.h`, `string.h`, `stdint.h`, and `stddef.h`;
- support normal installed include search paths.

### Later machine work
- machine JSON/configuration;
- `t32-node` multi-VM lifecycle;
- Foundry reporting/control integration;
- networking and richer interrupt-driven devices.

## Historical / conformance roadmap

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
