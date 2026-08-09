# T32 BIOS Contract

## Purpose

The T32 BIOS establishes early machine state, discovers basic platform facts,
loads the second stage, publishes Bootinfo, and provides a deliberately small
firmware service boundary.

## Current responsibilities

1. Execute at `0x00001000`.
2. Establish a temporary firmware stack.
3. Initialize the 80x25 text console.
4. Probe disk0 through the T32 disk MMIO contract.
5. Validate a T32D v0.1 boot disk.
6. Locate `BOOT.BIN` in the fixed T32D directory.
7. Discover RAM size through platform MMIO.
8. Construct Bootinfo v0.2 at `0x00002000`.
9. Load `BOOT.BIN` at `0x00010000`.
10. Publish BIOS service ABI v0.1, including `disk_read` at `0x00001008`.
11. Place the Bootinfo pointer in `r0` and transfer control to BOOT.BIN.

The exact Bootinfo layout and register contracts are defined in
`docs/abi/T32-BOOT-ABI.md`.

## Division of responsibility

BIOS knows the raw disk-controller MMIO protocol and T32D only far enough to
find BOOT.BIN. BOOT consumes Bootinfo and BIOS services. BOOT then uses
`disk_read` to parse T32D itself and load `NEXT.BIN` without touching raw disk
MMIO. NEXT.BIN receives Bootinfo but does not need to know T32D or the disk
controller at all.

```text
raw hardware/MMIO     BIOS
firmware services     BOOT.BIN
third-stage program   NEXT.BIN
```

This separation is intentional: the current third-stage code is now C-built
without moving hardware knowledge upward into every payload.
