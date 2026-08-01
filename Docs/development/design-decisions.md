# T32 Design Decisions

This document records decisions that must not remain implicit in code.

## Adopted

### 32-bit integer model

Registers and arithmetic results are 32 bits. Arithmetic wraps modulo 2^32.

### Four arithmetic flags

```text
C carry / no-borrow
Z zero
N negative
V signed overflow
```

### Subtraction carry means no borrow

```text
C=1  no borrow
C=0  borrow
```

### Register-based basic conditional branches

```asm
jz  ra, target
jnz ra, target
```

These instructions inspect a named register and do not consume arithmetic flags.

### Four-byte base instruction

The normal instruction word is four bytes.

## Provisional

### `r15` is the stack pointer

Current tests and the proposed ABI use `r15` as `sp`.

### Test images begin at `0x1000`

This is a validation convention, not yet the permanent reset address.

### Full-width immediate forms occupy eight bytes

Current binaries strongly indicate a four-byte instruction word followed by a 32-bit immediate.

## Open

- logical-instruction flag behavior;
- shift counts of 0, 32, and greater than 32;
- multiplication overflow visibility;
- signed division rounding;
- divide-by-zero and `INT32_MIN / -1` behavior;
- memory endianness and alignment;
- stack growth direction and frame layout;
- reset and exception vectors;
- trap and interrupt frame format;
- fixed MMIO regions.
