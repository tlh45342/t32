# T32 Conditional Branches

## Overview

T32 uses register-based conditional branches:

```asm
jz  ra, target
jnz ra, target
```

The branch names the register supplying the condition. Basic conditional control flow does not depend on implicit processor flags.

## Semantics

```text
JZ   branch when ra == 0
JNZ  branch when ra != 0
```

Neither instruction modifies the tested register, any other register, or memory.

## Loops

```asm
loop:
    ; work
    subi r2, r2, 1
    jnz  r2, loop
```

No separate compare is required.

## Equality comparison

To compare two registers, software explicitly produces a result:

```asm
xor r6, r4, r5
jz  r6, equal
```

If the values are equal, the XOR result is zero.

## Relationship to flags

`CMP` and `CMPI` may update arithmetic flags, but `JZ` and `JNZ` do not consume those flags.

Invalid current syntax:

```asm
jz target
jnz target
```

Valid syntax:

```asm
jz  r2, target
jnz r6, target
```

## Design rationale

Register-tested branches provide:

- explicit data dependencies;
- no stale-condition ambiguity;
- simple emulator behavior;
- predictable compiler generation;
- direct loop and counter handling.

The cost is occasional use of a temporary register. T32 accepts that cost in favor of explicit control flow.
