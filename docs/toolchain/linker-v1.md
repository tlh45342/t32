# t32-ld Version 1

## 1. Purpose

`t32-ld` combines T32OBJ files into a final flat binary.

Version 1 performs static linking only.

## 2. Basic command line

```text
t32-ld [options] object... -o output.bin
```

Example:

```text
t32-ld main.o strlen.o -o program.bin
```

## 3. Initial options

```text
-o FILE              output binary
-Ttext ADDRESS       .text load address
-Tdata ADDRESS       .data load address
-Map FILE            write link map
-e SYMBOL            entry symbol
-L DIRECTORY         add archive search directory
-l NAME              search libNAME.a
--version             print version
--help                print usage
```

Version 1 defaults:

```text
-Ttext 0x00001000
.data follows .text unless -Tdata is supplied
entry symbol = _start when present
```

The entry symbol is reported in the map file. A flat binary does not need a
special entry field.

## 4. Section placement

Default order:

```text
.text
.data
.bss
```

Placement algorithm:

1. Begin `.text` at `Ttext`.
2. Append each input object's `.text`, respecting alignment.
3. Place `.data` after `.text`, unless `Tdata` is explicit.
4. Append each input object's `.data`, respecting alignment.
5. Place `.bss` after `.data`, respecting alignment.
6. `.bss` contributes to addresses but not to output file bytes.

Input order is preserved.

## 5. Symbol resolution

The linker must:

- collect all global definitions;
- detect duplicate definitions;
- resolve undefined globals;
- reject unresolved globals;
- keep local symbols object-private;
- compute final addresses for defined symbols.

## 6. Relocation processing

After section placement, the linker applies every relocation.

For each relocation:

1. locate the target section bytes;
2. locate the referenced symbol;
3. compute `S + A`;
4. validate the result;
5. write the 32-bit little-endian value;
6. report an error when the patch lies outside the section.

## 7. Output binary

The output is a flat byte image beginning at the `.text` load address.

When `.data` follows `.text`, alignment gaps are filled with zeroes.

When `.data` is explicitly placed above `.text`, the output includes zero
padding between them.

Version 1 should reject overlapping output regions.

`.bss` is not written to the file.

## 8. Map file

Example:

```text
T32 Link Map

.text  0x00001000  0x00000054
.data  0x00001054  0x00000006
.bss   0x0000105C  0x00000020

Symbols:
0x00001000  GLOBAL  _start
0x00001020  GLOBAL  strlen
0x00001054  LOCAL   main.o:message

Entry:
0x00001000  _start
```

The map is diagnostic output and does not need a machine-readable format in
version 1.

## 9. Archive search

When `-l t32` is used, the linker searches for:

```text
libt32.a
```

Archive members are extracted only when they define a currently unresolved
global symbol.

Archive search proceeds left to right.

Version 1 may require repeating an archive on the command line when circular
dependencies exist.

## 10. Required diagnostics

Examples:

```text
error: undefined symbol 'strlen'
error: multiple definition of 'memcpy'
error: section placement overlap
error: relocation offset outside .text
error: relocation result does not fit in 32 bits
error: archive member has invalid T32OBJ format
```

Diagnostics should name the object file and symbol whenever possible.

## 11. Exit status

```text
0  successful link
1  link or input error
2  command-line usage error
```
