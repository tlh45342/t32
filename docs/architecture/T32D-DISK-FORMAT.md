# T32D Disk Format v0.1

Status: bootstrap format  
Sector size: 512 bytes  
Byte order: little-endian

## Purpose

T32D v0.1 gives native T32 media an unambiguous fingerprint and a filesystem-
independent way for early BIOS code to locate `BOOT.BIN`.

It is deliberately not GPT, EXT2, FAT, or a final T32 filesystem.

Future firmware may also support:

```text
GPT + EXT2 + /EFI/T32/BOOT.BIN
```

without changing the in-memory T32 Boot ABI.

## Layout

```text
LBA 0       header
LBA 1..4    directory
LBA 5..7    reserved
LBA 8..     file payloads
```

## Header — LBA 0

```text
offset  size  meaning
0x00    4     magic "T32D"
0x04    1     major version (0)
0x05    1     minor version (1)
0x06    2     reserved
0x08    4     sector size (512)
0x0C    4     total sectors
0x10    4     directory start LBA (1)
0x14    4     directory entry count (32)
0x18    4     directory entry size (64)
0x1C    4     first data LBA (8)
0x20    32    default boot filename ("BOOT.BIN")
0x40...        reserved
```

## Directory entry — 64 bytes

```text
offset  size  meaning
0x00    32    uppercase flat filename, NUL-terminated
0x20    4     starting LBA
0x24    4     exact byte length
0x28    4     flags
0x2C    20    reserved
```

Flag bit 0 marks the conventional boot payload.

Files are contiguous in v0.1. There are no subdirectories, free-space bitmap,
permissions, timestamps, fragmentation, or deletion/compaction semantics yet.

## BIOS boot convention

Early BIOS should:

1. read LBA 0;
2. require magic `T32D`;
3. validate version/sector size;
4. scan the fixed directory for `BOOT.BIN`;
5. read its contiguous sectors;
6. copy exactly `byte_length` bytes to `0x00010000`;
7. transfer control to `0x00010000`.
