# T32 Memory Map

## Current test environment

The supplied tests use these addresses:

| Address | Current use | Status |
|---:|---|---|
| `0x00001000` | Program load address and entry point | Test convention |
| `0x00002000` | Scratch data address for load/store tests | Test convention |
| `0x00003000` | Initial stack pointer in stack/call tests | Test convention |

These addresses describe the present validation environment. They are not yet a complete architectural memory map.

## Program loading

Tests perform:

```text
load <program>.bin 0x1000
set pc 0x1000
```

Assembly sources use:

```asm
.org 0x1000
```

This pairing makes code labels and runtime addresses agree.

## Data memory

Load/store smoke tests use `0x2000` as writable data memory. They verify byte, halfword, and word round trips.

The following still require specification:

- address-space size;
- valid RAM ranges;
- read-only regions;
- unmapped-access behavior;
- alignment requirements;
- byte order;
- memory protection;
- executable versus non-executable regions.

## Stack

Stack tests initialize `r15` to `0x3000`.

The tests verify stack restoration after PUSH/POP, CALL/RET, and IRET sequences, but this document does not yet claim:

- whether the stack grows upward or downward;
- whether SP points to the top element or next free location;
- stack alignment;
- interrupt-frame layout.

These must be extracted from implementation behavior and locked down with dedicated tests.

## Planned architectural regions

A future formal map should define at least:

```text
exception vectors / reset vector
boot ROM
program ROM or loaded image region
RAM
stack convention
memory-mapped video
memory-mapped timer and interrupt controller
other device I/O
reserved address ranges
```

The memory map should be versioned before C runtime and operating-system work depends on fixed addresses.
