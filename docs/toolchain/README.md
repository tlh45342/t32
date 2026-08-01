# T32 Toolchain Documentation

This directory defines the first relocatable-object and static-linking model
for the T32 toolchain.

The implementation order is:

```text
1. T32OBJ v1 specification
2. t32-as -f obj
3. t32-nm
4. t32-ld
5. t32-ar
6. libt32.a
```

The documents are intentionally conservative. T32OBJ v1 supports only the
features required to assemble separate modules, inspect symbols, resolve
references, link flat binaries, and later package object files in a static
archive.

## Documents

- `object-format-v1.md`
  - Binary layout of a `.o` file.
- `assembler-output-formats.md`
  - `t32-as` behavior for `-f bin` and `-f obj`.
- `symbols-and-relocations.md`
  - Symbol visibility, undefined references, and relocation semantics.
- `linker-v1.md`
  - First `t32-ld` command line, placement rules, diagnostics, and map output.
- `archive-v1.md`
  - Initial `t32-ar` behavior and `libt32.a` member-selection rules.
- `first-linked-program.md`
  - End-to-end proof using `main.o` and `strlen.o`.

## Non-goals for version 1

T32OBJ v1 does not provide:

- dynamic linking;
- shared libraries;
- weak symbols;
- symbol versioning;
- debug information;
- section groups;
- link-time optimization;
- exception-unwind metadata;
- position-independent code requirements;
- ELF compatibility.

Those can be considered later without changing the purpose of this first
format.
