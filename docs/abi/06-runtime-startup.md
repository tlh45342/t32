# 06 — Runtime Startup

## 1. Purpose

The runtime startup object, conventionally named:

```text
crt0.o
```

bridges machine entry and a C program.

## 2. Initial responsibilities

The first `crt0` should:

1. establish or validate `r15`;
2. initialize `.bss` to zero;
3. prepare process arguments when supported;
4. call `main`;
5. terminate through HALT or an exit service.

## 3. Minimal form

```asm
.section .text
.global _start
.extern main

_start:
    movi r15, 0x0000F000
    call main
    halt
```

## 4. `.bss`

A complete runtime must zero the linked `.bss` range before calling C code.

The linker or linker-generated symbols should eventually expose:

```text
__bss_start
__bss_end
```

## 5. `.data`

For a directly loaded flat binary, initialized `.data` is already present in the loaded image.

A ROM-to-RAM startup environment may require copying `.data` from its load location to its execution location.

That behavior is platform-specific and not required by the first hosted runtime.

## 6. Constructors

Version 0.1 does not define global constructors or destructors.

## 7. Exit

The initial standalone runtime may use:

```asm
halt
```

A later hosted runtime should define:

```c
void exit(int status);
```

with the status supplied in `r0`.
