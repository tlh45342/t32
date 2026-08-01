# T32 Memory-Mapped I/O

## Status

This document reserves the shape of the platform contract. Exact addresses and register layouts remain provisional.

## Design rule

Guest software sees stable MMIO devices. Host implementations may map those devices onto VCONSOLE, VDISK, files, sockets, viewers, or other services.

The guest contract must not expose unnecessary host implementation details.

## Planned device classes

```text
console control and output
text framebuffer
keyboard or input queue
system timer
interrupt controller
RTC
virtual disk
network interface
platform identification
```

## Device specification template

Each device must define:

- base address and region size;
- supported access widths;
- register offsets;
- reset values;
- read/write behavior;
- status and error bits;
- interrupt source and acknowledgement;
- ordering requirements;
- unsupported-access behavior.

## Early console

The first console device should support at least one simple character-output path suitable for BIOS diagnostics. A memory-mapped text framebuffer may be added separately for direct 80x25 display access.
