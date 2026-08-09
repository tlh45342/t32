# T32 BOOT 0.0.4

`BOOT.BIN` is the T32 second-stage loader and Bootinfo v0.2 consumer.

```text
load address : 0x00010000
entry point  : 0x00010000
r0           : pointer to Bootinfo v0.2
```

BOOT establishes its own stack, validates the 72-byte Bootinfo record, and can
run standalone when BIOS services are absent. When Bootinfo flag bit1 says the
BIOS service ABI is available, BOOT uses `disk_read` at `0x00001008` instead
of touching disk MMIO directly.

## Current boot path

```text
BIOS
  -> load BOOT.BIN @ 0x00010000
  -> r0 = Bootinfo v0.2
BOOT.BIN
  -> validate Bootinfo
  -> validate BIOS disk service
  -> disk_read T32D header into RAM
  -> scan T32D directory for NEXT.BIN
  -> disk_read NEXT.BIN directly to 0x00020000
  -> r0 = Bootinfo v0.2
  -> jump 0x00020000
```

The guest filename is currently fixed as `NEXT.BIN`. The third-stage load and
entry address are both `0x00020000`.

A successful integrated boot leaves the visible checkpoints:

```text
T32 BOOT 0.0.4
BOOTINFO v0.2 OK
BIOS DISK SERVICE v0.1 OK
Hello from BOOT.BIN
T32 STAGE3 0.0.1
NEXT.BIN loaded by BOOT.BIN
Bootinfo v0.2 handoff OK
```

BOOT reports `NEXT.BIN NOT FOUND` if the boot disk is valid and BOOT.BIN runs,
but the third-stage payload is absent.
