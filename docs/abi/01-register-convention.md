# 01 — Register Convention

## 1. Register classes

| Registers | Role | Preservation |
|---|---|---|
| `r0-r3` | arguments and return values | caller-saved |
| `r4-r7` | temporary and scratch values | caller-saved |
| `r8-r14` | preserved general-purpose values | callee-saved |
| `r15` | stack pointer | restored by callee |

## 2. Argument registers

The first four scalar or pointer arguments are passed in order:

```text
argument 1 -> r0
argument 2 -> r1
argument 3 -> r2
argument 4 -> r3
```

Additional arguments are passed on the stack.

## 3. Return registers

The primary scalar or pointer return value is placed in:

```text
r0
```

A future 64-bit scalar result may use:

```text
r0:r1
```

with `r0` containing the low 32 bits and `r1` the high 32 bits.

Version 0.1 does not standardize aggregate returns.

## 4. Caller-saved registers

A caller must assume that a function call may change:

```text
r0-r7
```

If the caller needs one of those values after the call, it must preserve the value before calling.

## 5. Callee-saved registers

A callee that changes any of:

```text
r8-r14
```

must restore the original value before returning.

A function that does not touch those registers does not need to save them.

## 6. Stack pointer

`r15` is the stack pointer.

A conforming function shall:

- treat `r15` as the current top of stack;
- preserve 4-byte alignment at public call boundaries;
- restore `r15` to its entry value before `RET`, except when returning from a deliberately nonstandard control transfer.

## 7. Register aliases

Tooling may later support symbolic aliases such as:

```text
sp -> r15
```

The ABI is defined in terms of numbered registers and does not require assembler aliases.

## 8. Example

```asm
.section .text
.global add_three

; int add_three(int a, int b, int c)
;
; r0 = a
; r1 = b
; r2 = c
; returns r0 = a + b + c
;
add_three:
    add  r0, r0, r1
    add  r0, r0, r2
    ret
```
