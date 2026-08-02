# 04 — Process Entry

## 1. Entry symbol

The conventional program entry symbol is:

```text
_start
```

## 2. Initial machine state

At `_start`, the runtime shall provide:

```text
r15 = valid 4-byte-aligned stack pointer
r0  = boot or process information pointer, or zero
r1  = argument count, initially zero when unsupported
r2  = argument vector pointer, initially zero when unsupported
r3  = environment pointer, initially zero when unsupported
```

Registers not listed above are unspecified unless the platform contract defines them.

## 3. Minimal entry

```asm
.section .text
.global _start
.extern main

_start:
    movi r15, 0x0000F000
    call main
    halt
```

## 4. Return from main

The C `main` return value is received in `r0`.

The first standalone runtime may terminate with `HALT`.

A hosted runtime may instead call an exit service.

## 5. Entry ownership

The application normally defines `main`.

The runtime normally defines `_start`.

A user program should not define `_start` when it links against `crt0.o`.

## 6. Future process arguments

When process arguments exist:

```text
r1 = argc
r2 = argv
r3 = envp
```

The initial C startup routine may translate those into the implementation's selected `main` signature.
