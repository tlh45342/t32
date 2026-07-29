# T32 Instruction Encoding

## Current evidence

The supplied tests establish several facts:

1. The normal instruction width is four bytes.
2. `HALT` advances the PC from `0x1000` to `0x1004`.
3. Instructions that carry a full 32-bit immediate or address may occupy additional bytes.
4. Expanded arithmetic tests verify exact four-byte encodings at known offsets.

For example, expanded ADD validation expects:

```text
add r2, r0, r1  -> 00 10 20 18
```

The byte sequence is written in file order. Its final byte is `0x18`, matching decimal opcode 24.

An alias case expects:

```text
add r0, r0, r1  -> 00 10 00 18
```

This supports a provisional register-register layout in which register fields occupy nibbles or bytes before the opcode byte.

## Provisional 32-bit register form

Based on the checked ADD encodings, a useful working representation is:

```text
byte 0     reserved / function extension
byte 1     source register B and/or packed register field
byte 2     destination/source register fields
byte 3     opcode
```

However, the exact bit allocation must be derived from the assembler/runtime implementation and documented with multiple instruction examples before this becomes normative.

## Immediate and target forms

Current assembly demonstrates full-width values:

```asm
movi r0, 0xffffffff
movi r0, 0x80000000
jmp target
call function
```

Expanded test binaries place ADD after two MOVI instructions at byte offset 16, implying each MOVI occupies eight bytes in the current binary format:

```text
MOVI: base instruction word + 32-bit immediate
```

This is strong evidence for a double-word immediate form, but the exact field layout remains to be documented from source.

## Encoding requirements for conformance tests

Expanded instruction tests should verify:

- exact opcode byte;
- destination register field;
- source register fields;
- immediate value and byte order;
- label/target relocation or absolute value;
- alias encodings;
- reserved bits are zero unless explicitly assigned.

## Endianness

The exact architectural memory endianness is not fully established by the supplied documentation.

The expanded tests compare raw binary bytes in file order. Memory tests currently prove round-trip store/load behavior but do not independently prove byte ordering for multi-byte values.

**Required next step:** add memory tests that store a known word and inspect individual bytes, then record the architectural endianness here.

## Future extension policy

Encoding changes should follow these rules:

1. Existing assigned opcodes remain stable once the ISA is declared versioned.
2. Reserved bits must have defined behavior, preferably required to be zero.
3. New instruction forms should use unused opcode ranges or a documented extension mechanism.
4. The assembler and runtime must reject malformed or unsupported encodings consistently.
5. Every encoding form must have at least one exact-byte conformance test.
