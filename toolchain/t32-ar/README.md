# t32-ar 0.0.1

`t32-ar` packages T32OBJ files into deterministic static archives.

## Commands

```text
t32-ar rcs libt32.a file.o ...
t32-ar t   libt32.a
t32-ar x   libt32.a [member.o ...]
t32-ar d   libt32.a member.o ...
```

## Archive format

Version 1 uses a T32-specific binary format with:

- `T32AR` magic and version;
- member table;
- global-symbol index;
- string table;
- member data.

Only valid T32OBJ members are accepted.

## Build

```text
make
make test
make install
```

## Current boundary

`t32-ar 0.0.1` creates and maintains archives.

`t32-ld` must be upgraded separately before it can consume `libt32.a`.
