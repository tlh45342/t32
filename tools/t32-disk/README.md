# t32-disk 0.0.1

`t32-disk` is the host-side reference utility for T32-native disk images.

It belongs under `tools/`, not `toolchain/`: it manages virtual media rather
than translating source code.

## T32D v0.1

The first format is intentionally tiny and BIOS-friendly:

```text
LBA 0       T32D header / fingerprint
LBA 1..4    fixed directory (32 entries x 64 bytes)
LBA 5..7    reserved
LBA 8..     contiguous file data
```

The header begins with the ASCII magic:

```text
T32D
```

and records:

- format version 0.1
- 512-byte sector size
- total sector count
- directory location/count/entry size
- first data LBA
- default boot filename `BOOT.BIN`

This is a bootstrap media format, not the eventual general-purpose filesystem.

## Commands

```text
create <image> <size>
format <image>
info <image>
list <image>
put <image> <hostfile> <name>
get <image> <name> <hostfile>
do <script>
help
quit
```

Guest filenames are currently flat, case-normalized names such as `BOOT.BIN`.
There are no directories yet.

## Interactive mode

```text
t32-disk
t32-disk> create disk.img 16M
t32-disk> format disk.img
t32-disk> put disk.img boot.bin BOOT.BIN
t32-disk> list disk.img
```

## Automation

The same parser is used for redirected stdin:

```bat
t32-disk < create-boot.script
```

and script execution:

```text
t32-disk> do create-boot.script
```

or:

```bat
t32-disk do create-boot.script
```

Redirected input and `do` scripts are fail-fast and return nonzero on the first
failed command.

## First boot-image workflow

After `firmware/boot/boot.bin` has been built:

```text
create disk.img 16M
format disk.img
put disk.img ../../firmware/boot/boot.bin BOOT.BIN
info disk.img
list disk.img
```

The next BIOS milestone will recognize this T32D header, locate `BOOT.BIN`,
load it at `0x00010000`, and transfer control there.
