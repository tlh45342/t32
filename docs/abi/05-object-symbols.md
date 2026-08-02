# 05 — Object and Symbol Convention

## 1. Public symbols

A routine intended for use outside its object file must be declared global:

```asm
.section .text
.global strlen

strlen:
    ...
```

## 2. Local symbols

Internal labels remain local by default:

```asm
strlen_loop:
strlen_done:
```

Local labels may reuse the same spelling in different object files.

## 3. External references

A module declares an externally defined symbol with:

```asm
.extern strlen
```

An unresolved external must be satisfied by another object file or an extracted archive member.

## 4. Sections

Version 0.1 uses:

```text
.text
.data
.bss
```

Recommended purposes:

```text
.text  executable instructions
.data  initialized writable objects
.bss   zero-initialized writable objects
```

## 5. Function symbols

Public function names should be plain identifiers:

```text
strlen
memcpy
main
_start
```

Version 0.1 defines no leading underscore decoration.

## 6. Object symbols

Writable data objects use `.data` or `.bss`.

```asm
.section .data
.global counter

counter:
    .word 0
```

## 7. Archive naming

The standard T32 runtime archive is:

```text
libt32.a
```

The linker should extract only members needed to satisfy unresolved global symbols.

## 8. Symbol ownership

One final link may contain at most one global definition of a given symbol.

Multiple definitions are a link error.
