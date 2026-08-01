# T32 Symbols and Relocations

## 1. Symbol model

Every named label or constant is represented as one of:

```text
local defined
global defined
global undefined
absolute
```

Examples:

```asm
.local_label:
.global strlen
.extern puts
.equ STACK_TOP, 0xF000
```

T32 source does not require a `.local` directive. A symbol is local by
default.

## 2. Duplicate definitions

Within one object:

```text
two definitions of the same symbol are an assembler error
```

Across objects:

```text
two global definitions of the same symbol are a linker error
```

Multiple local symbols with the same spelling in different object files are
permitted.

## 3. Undefined symbols

An undefined global symbol is legal in an object file.

It must be resolved by:

- another directly supplied object file; or
- an extracted archive member.

Any remaining undefined symbol is a final-link error.

## 4. Relocation types

T32OBJ v1 defines three relocation types.

### 4.1 `R_T32_ABS32`

```text
value = S + A
```

Where:

```text
S = final symbol address
A = signed relocation addend
```

The linker writes the 32-bit little-endian result at the relocation offset.

Primary uses:

- `.word symbol`;
- absolute address constants;
- address fields in multiword instructions.

### 4.2 `R_T32_TARGET32`

```text
value = S + A
```

The linker patches an absolute control-flow target.

Primary uses:

```asm
call function
jmp  label
jz   r0, label
jnz  r0, label
```

The relocation points at the four-byte target field, not at the opcode word.

This preserves the existing absolute-target model.

### 4.3 `R_T32_ADDR32`

```text
value = S + A
```

The linker patches an address operand loaded by an instruction such as:

```asm
movi r0, message
```

Version 1 keeps this distinct from `ABS32` so diagnostics and future encoding
changes can distinguish instruction address operands from raw data words.

## 5. Overflow rules

All version 1 relocation results must fit in 32 bits.

The linker rejects a value outside:

```text
0x00000000 through 0xFFFFFFFF
```

No truncation is permitted.

## 6. Addends

The addend is stored in the relocation entry.

The section bytes at the patch location should contain zero before linking.

Example:

```asm
.word message + 4
```

may become:

```text
symbol = message
addend = 4
type   = R_T32_ABS32
```

General expression support is not required in the first assembler
implementation. Supporting `symbol + integer` is sufficient.

## 7. Relocation ownership

Relocations belong to the section containing the bytes to patch.

Example:

```text
.text relocation references .data symbol
```

is valid.

A relocation may reference:

- a local symbol;
- a global defined symbol;
- a global undefined symbol.

## 8. Local-symbol resolution

A relocation referencing a local symbol must be resolved within the same
object file.

The linker must never search other objects for a local symbol.

## 9. Global-symbol resolution

The linker builds one global symbol table from all input objects and extracted
archive members.

Resolution rules:

1. One definition satisfies all references.
2. Zero definitions is an undefined-symbol error.
3. More than one definition is a duplicate-symbol error.

## 10. Control-flow target model

T32OBJ v1 assumes T32 control-flow instructions store absolute 32-bit target
addresses.

Therefore:

```text
CALL/JMP/JZ/JNZ targets use R_T32_TARGET32
```

A future PC-relative encoding would require a new relocation type rather than
changing the meaning of version 1.

## 11. Example

Source:

```asm
.global _start
.extern strlen

.section .text

_start:
    movi r0, message
    call strlen
    halt

.section .data

message:
    .byte 'H', 'e', 'l', 'l', 'o', 0
```

Expected relocation concepts:

```text
.text address field for message -> R_T32_ADDR32
.text call target for strlen    -> R_T32_TARGET32
```
