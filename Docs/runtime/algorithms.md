# T32 Runtime Algorithms

## Purpose

The algorithm validation suite proves that T32 can support practical runtime operations above the raw instruction level.

## Current suite

| Directory | Runtime analogue | Status |
|---|---|---|
| `00-memory-fill` | `memset` | Validated |
| `01-memory-copy` | `memcpy` | Validated |
| `02-memory-compare` | `memcmp` | Validated |
| `03-string-length` | `strlen` | Validated |
| `04-string-compare` | `strcmp`-style equality/difference | Validated |
| `05-string-copy` | `strcpy` | Validated |
| `06-string-to-int` | unsigned decimal parse | Validated |
| `07-int-to-string` | unsigned decimal formatting | Validated |
| `08-string-reverse` | in-place string reversal | Validated |
| `09-hex-to-string` | hexadecimal formatting | Planned |
| `10-string-to-hex` | hexadecimal parsing | Planned |

## Standard routine documentation

Each promoted runtime routine should document:

- purpose;
- arguments;
- return value;
- clobbered registers;
- preserved registers;
- memory effects;
- error behavior;
- complexity;
- example call.

## Promotion path

```text
algorithm validation
        ↓
shared assembly routine
        ↓
libt32 runtime
        ↓
C declaration
        ↓
compiler-generated call
        ↓
optional rewrite in C
```

The tests should ultimately consume shared runtime implementations rather than private embedded copies.
