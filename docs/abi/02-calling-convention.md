# 02 — Calling Convention

## 1. Public function call

A public call uses:

```asm
call function
```

The callee returns with:

```asm
ret
```

`CALL` and `RET` use the machine stack for the return address.

## 2. Argument order

The first four arguments are assigned left to right:

```c
f(a, b, c, d)
```

becomes:

```text
r0 = a
r1 = b
r2 = c
r3 = d
```

## 3. Stack arguments

Arguments beyond the fourth are pushed right to left.

For:

```c
f(a, b, c, d, e, f)
```

the caller prepares:

```text
push f
push e
r0 = a
r1 = b
r2 = c
r3 = d
call f
```

The caller removes stack arguments after return.

This is a cdecl-like cleanup model.

## 4. Return value

The callee places its primary result in `r0`.

Example:

```asm
.section .text
.global identity

identity:
    ; r0 already contains the return value
    ret
```

## 5. Caller responsibilities

Before a call, the caller must:

- place register arguments in `r0-r3`;
- place additional arguments on the stack;
- preserve any required values currently in `r0-r7`;
- ensure the stack is 4-byte aligned.

After a call, the caller must:

- read the return value from `r0`;
- remove any stack arguments it supplied;
- assume `r0-r7` may have changed.

## 6. Callee responsibilities

The callee must:

- preserve `r8-r14` when it changes them;
- restore `r15`;
- return with `RET`;
- place the result in `r0`.

## 7. Leaf function

A leaf function that uses only caller-saved registers may require no stack frame.

```asm
.section .text
.global square

square:
    mul  r0, r0, r0
    ret
```

## 8. Non-leaf function

```asm
.section .text
.global wrapper
.extern helper

wrapper:
    push r8
    mov  r8, r0

    call helper

    add  r0, r0, r8

    pop  r8
    ret
```
