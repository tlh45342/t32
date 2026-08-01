# T32 Application Binary Interface

## Status

This ABI is an initial software contract for assembly routines, BIOS code, and the future C toolchain. It is **provisional** until exercised by linked multi-module programs.

## Register roles

| Register | Provisional role |
|---|---|
| `r0-r3` | Arguments and return values |
| `r4-r11` | General working registers |
| `r12-r14` | Reserved general/temporary use |
| `r15` | Stack pointer (`sp`) |

## Function calls

`CALL` transfers control to a function and records a return address on the stack. `RET` restores that address and returns.

### Arguments

The first four scalar arguments should be passed in:

```text
r0, r1, r2, r3
```

Additional arguments should be passed on the stack once stack layout is frozen.

### Return values

A scalar return value should be placed in `r0`.

A second scalar or status value may use `r1` when required by a documented routine contract.

## Register preservation

**Provisional convention:**

- caller-saved: `r0-r3`, `r12-r14`;
- callee-saved: `r4-r11`;
- stack pointer: `r15` must be restored before return.

Leaf routines may avoid stack use.

## Stack

The exact growth direction, alignment, frame layout, and argument spill layout remain open and must be frozen before C interoperability.

## Variadic functions

Variadic calling rules are not yet defined. A tiny `printf` should initially use a deliberately narrow private convention rather than claim general C ABI compatibility.

## Interrupts

Interrupt and exception frames are not ordinary function frames. Their format belongs in the future exception/interrupt specification.
