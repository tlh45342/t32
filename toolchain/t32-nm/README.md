# t32-nm 0.0.1

`t32-nm` inspects T32OBJ v1 relocatable object files produced by
`t32-as -f obj`.

## Build

```text
make
make test
make install
```

## Usage

```text
t32-nm file.o
t32-nm --sections file.o
t32-nm --symbols file.o
t32-nm --relocs file.o
t32-nm --version
```

With no view option, all three tables are displayed.

## Symbol letters

```text
T/t  executable section, global/local
D/d  writable initialized section, global/local
B/b  NOBITS section, global/local
A/a  absolute symbol, global/local
U    undefined global
```

## Scope

Version 0.0.1 reads and validates:

- T32OBJ v1 headers;
- section tables;
- symbol tables;
- relocation tables;
- string tables;
- `R_T32_ABS32`, `R_T32_TARGET32`, and `R_T32_ADDR32`.

It does not disassemble instructions or modify object files.
