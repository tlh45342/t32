# T32 object-format next step

`libt32 0.0.1` is intentionally source-only.

The next toolchain milestone is:

```text
t32-as -f obj source.s -o source.o
t32-nm source.o
t32-ld main.o source.o -o program.bin
t32-ar rcs libt32.a *.o
```

The first object format should preserve:

- section bytes;
- defined symbols;
- undefined symbols;
- relocation entries;
- a string table;
- section alignment.

The existing `-f bin` behavior should remain compatible and explicit in
project Makefiles.
