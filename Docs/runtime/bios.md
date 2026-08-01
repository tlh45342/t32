# T32 BIOS Contract

## Purpose

The first T32 BIOS should be intentionally small. Its job is to establish a known machine state, initialize essential devices, report basic diagnostics, and transfer control to a boot image.

## Initial responsibilities

1. Enter at the architectural reset vector.
2. Initialize the stack.
3. Establish exception and interrupt vectors.
4. Initialize the console.
5. Print a startup banner.
6. Discover or verify RAM.
7. Probe required platform devices.
8. construct boot information.
9. Load or locate the boot image.
10. Transfer control.

## Example output

```text
Foundry BIOS 0.0.1
CPU      : T32
RAM      : 64 KB
Console  : OK
Timer    : OK
Disk     : OK

Booting...
```

## Boot information

A future boot-information structure should include at least:

```text
memory size and usable ranges
console descriptor
timer frequency
RTC availability
storage devices
network devices
firmware version
command line or boot parameters
```

## Non-goals for the first BIOS

- filesystem support;
- rich menus;
- dynamic driver loading;
- full hardware abstraction;
- general-purpose services after the operating environment takes control.
