# T32 Design Decisions

This document records decisions that should not be left implicit in code. A decision may be marked **Adopted**, **Provisional**, or **Open**.

## Adopted: 32-bit integer register model

T32 arithmetic and register reporting use 32-bit values. Arithmetic wraps modulo 2^32.

Evidence: expanded arithmetic tests verify results such as `0xffffffff + 1 = 0`.

## Adopted: four arithmetic flags

The current arithmetic model uses:

```text
C carry / no-borrow
Z zero
N negative (bit 31)
V signed overflow
```

Expanded ADD, ADDI, SUB, SUBI, CMP, and CMPI tests define and verify these semantics.

## Adopted: subtraction carry means no borrow

For subtraction and comparison:

```text
C=1  no borrow
C=0  borrow occurred
```

This convention is already embedded in expanded tests and should be treated as intentional unless deliberately revised.

## Provisional: r15 is the stack pointer

Current PUSH/POP/CALL/RET/IRET tests initialize `r15` as the stack pointer.

Decision required: formally reserve `r15` as SP and optionally add an assembler alias `sp`, or state that stack instructions implicitly use a configurable/general register.

## Provisional: test image starts at 0x1000

All current tests assemble and load at `0x1000`.

This is a test and toolchain convention, not yet a permanent reset vector or executable-memory rule.

## Provisional: full-width immediate form is eight bytes

Expanded arithmetic binaries place a register instruction at offset 16 after two MOVI instructions, strongly indicating an eight-byte MOVI form.

The exact encoding must be confirmed from assembler/runtime source.

## Open: logical-instruction flag behavior

Choose and document one rule for AND, OR, XOR, and NOT:

- update N and Z while preserving C and V;
- update N and Z while clearing C and V;
- update all flags by another defined rule;
- do not modify flags.

Tests should then enforce that decision.

## Open: shift-count behavior

Define behavior for counts:

```text
0
1-31
32
greater than 32
```

Possible models include masking the count, saturating behavior, defined zero/sign-fill results, or trapping. Carry-out for count zero must also be explicit.

## Open: multiplication result and flags

Define:

- whether MUL is signed and MULU unsigned only for overflow interpretation or for result calculation;
- whether only the low 32 bits are retained;
- whether overflow is observable;
- which flags change.

## Open: division exceptions

Define:

- truncation direction for signed division;
- divide-by-zero behavior;
- `0x80000000 / -1` behavior;
- destination state on fault;
- which flags change.

## Open: memory byte order and alignment

Round-trip tests do not independently establish endianness. Add byte-inspection tests and define misaligned-access behavior.

## Open: branch-condition model

Current JZ/JNZ syntax tests a register directly:

```asm
jz r0, target
jnz r0, target
```

Decide whether T32 will also need flag-based conditional branches after CMP/CMPI, or whether register-tested branches are the intended minimal model.

## Open: exception and interrupt frame

IRET and TRAP exist, but the architectural frame layout, vector model, saved flags, return PC, and nesting rules need a formal decision before interrupt-driven operating-system work.
