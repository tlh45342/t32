# T32 BIOS 0.0.6

T32 BIOS 0.0.6 performs the T32D disk bootstrap, constructs Bootinfo v0.2,
and exposes BIOS service ABI v0.1.

The BIOS executes at `0x00001000`, talks to disk0 through MMIO, finds
`BOOT.BIN`, builds Bootinfo at `0x00002000`, loads BOOT.BIN at `0x00010000`,
places the Bootinfo pointer in `r0`, and transfers control.

## Firmware boundary

```text
BIOS @ 0x00001000
  -> validate T32D v0.1
  -> discover RAM size
  -> build Bootinfo v0.2 @ 0x00002000
  -> load BOOT.BIN @ 0x00010000
  -> r0 = 0x00002000
  -> jump BOOT.BIN

BIOS service ABI v0.1
  0x00001008  disk_read(disk, LBA, destination)
```

`disk_read` copies exactly one 512-byte sector into caller-supplied guest RAM.
BOOT.BIN uses that service to locate and load `NEXT.BIN`; later stages therefore
do not need the raw disk-controller buffer protocol.

The exact Bootinfo and BIOS service contracts are defined in
`docs/abi/T32-BOOT-ABI.md`.

## End-to-end chain

```text
BIOS -> T32D -> BOOT.BIN -> BIOS disk_read -> NEXT.BIN
```

The BIOS validation creates real T32D images, checks missing/invalid media,
checks the BOOT-only failure case where `NEXT.BIN` is absent, installs both
BOOT.BIN and NEXT.BIN, and proves execution reaches the third stage.

The current `NEXT.BIN` milestone is linked from a Stage-3 assembly startup shim plus a `t32-cc` generated C `main()`; BIOS and BOOT require no changes for that transition.
