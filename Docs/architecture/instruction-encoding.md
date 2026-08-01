# T32 Instruction Encoding

## Base width

The normal instruction word is four bytes.

Some instructions carry an additional 32-bit immediate or target and therefore occupy eight bytes in the current binary format.

## Current evidence

Expanded tests verify exact encodings for several arithmetic and comparison instructions. For example:

```text
add r2, r0, r1  -> 00 10 20 18
```

The final byte contains opcode `0x18` (decimal 24).

## Provisional register form

The current binary layout appears to encode register fields before the opcode byte:

```text
byte 0    reserved or extension field
byte 1    source register field
byte 2    destination/source register fields
byte 3    opcode
```

This representation remains provisional until derived completely from assembler and runtime source.

## Immediate and target forms

`MOVI`, and likely full-width branch/call target forms, use:

```text
base instruction word
32-bit immediate or target
```

The immediate byte order must be documented by exact-byte tests.

## Conformance requirements

Each encoding form should validate:

- opcode;
- destination and source fields;
- immediate or target width;
- immediate byte order;
- alias encodings;
- reserved bits;
- rejection of malformed encodings.

## Stability policy

Once an ISA version is published:

1. assigned opcodes remain stable;
2. reserved bits have defined behavior;
3. new forms use unassigned ranges or a documented extension mechanism;
4. assembler and runtime reject unsupported forms consistently;
5. every form has at least one exact-byte test.
