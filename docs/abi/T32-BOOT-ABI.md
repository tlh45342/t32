# T32 Boot ABI v0.2

## Fixed addresses

```text
0x00001000   BIOS reset entry
0x00001008   BIOS service v0.1: disk_read
0x00002000   Bootinfo v0.2 record
0x00010000   BOOT.BIN load address and entry point
```

BIOS locates `BOOT.BIN`, copies it to `0x00010000`, constructs Bootinfo at
`0x00002000`, places `0x00002000` in `r0`, and transfers control to
`0x00010000`.

## Entry register contract

At BOOT.BIN entry:

```text
r0 = address of Bootinfo v0.2
```

All other general-purpose registers are unspecified. BOOT.BIN establishes its
own stack.

## Bootinfo v0.2

Bootinfo v0.2 is a fixed 72-byte little-endian record.

| Offset | Field | v0.2 meaning |
|---:|---|---|
| `0x00` | magic | `0x42323354` (`T32B` in memory) |
| `0x04` | size | structure size, `72` |
| `0x08` | version_major | `0` |
| `0x0c` | version_minor | `2` |
| `0x10` | flags | bit0 boot disk valid; bit1 BIOS services available |
| `0x14` | ram_base | usable RAM base; currently `0` |
| `0x18` | ram_size | usable RAM bytes discovered by BIOS |
| `0x1c` | boot_disk | boot disk number; currently `0` |
| `0x20` | sector_size | boot disk sector size |
| `0x24` | sector_count | boot disk sector count |
| `0x28` | text_base | text framebuffer MMIO base |
| `0x2c` | text_columns | text framebuffer columns |
| `0x30` | text_rows | text framebuffer rows |
| `0x34` | disk_mmio_base | boot disk controller MMIO base |
| `0x38` | platform_mmio_base | platform-control MMIO base |
| `0x3c` | boot_entry | address BIOS transferred to |
| `0x40` | bios_service_version | `1` means BIOS service ABI v0.1 |
| `0x44` | bios_disk_read | fixed entry `0x00001008` when flag bit1 is set |

Consumers validate `magic`, `size`, and version before interpreting fields.
BIOS services are optional: if flags bit1 is clear, a v0.2 consumer must not
call the service entry.

## BIOS service ABI v0.1

The first firmware service is intentionally tiny and synchronous.

### disk_read — `0x00001008`

```text
input:
  r0 = disk number (0 only)
  r1 = LBA
  r2 = destination RAM address

return:
  r0 = 0 success
  r0 = 1 error

preserved:
  r8-r14
  r15 is restored by CALL/RET

volatile:
  r0-r7
```

Exactly one 512-byte sector is copied from the selected disk into ordinary
guest RAM. The caller does not need to know the disk MMIO buffer protocol.

The fixed entry is possible because the BIOS reset location contains one
8-byte `JMP` over the service routine, making `0x00001008` stable without a
linker. Tests enforce this contract.

## Stage-3 handoff v0.1

BOOT.BIN currently recognizes the fixed T32D guest filename `NEXT.BIN`, loads
it at `0x00020000`, and transfers control to the same address.

```text
r0 = pointer to Bootinfo v0.2
all other general-purpose registers = unspecified
```

NEXT.BIN must establish its own stack. The initial demonstration uses
`0x0000E000` and validates the Bootinfo magic/size before continuing.

This is intentionally a small seam rather than a final executable format. A
future C-built third stage can replace the assembly payload without changing
the BIOS disk service or BOOT's media-loading responsibility.
