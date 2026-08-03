# T32 crt0 0.0.1

`crt0.o` is the first T32 C runtime startup object.

It defines the ABI 0.1 entry symbol `_start`, establishes the initial stack,
calls an external `main`, and halts with `main`'s return value still in `r0`.

## Runtime path

```text
_start
  -> r15 = 0x0000F000
  -> call main
  -> r0 contains main result
  -> HALT
```

## Build contract

```text
make
make test
make install
make clean
```

The repository-local object is:

```text
build/crt0.o
```

Installation places it under:

```text
~/.local/lib/t32/crt0.o
```

On Windows this is normally:

```text
C:\Users\<user>\.local\lib\t32\crt0.o
```

## Scope

Version 0.0.1 deliberately does not initialize `.bss`, copy `.data`, construct
`argc`/`argv`, or call an exit service. Those belong to later runtime stages.
