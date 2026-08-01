# t32-as Output Formats

## 1. Purpose

`t32-as` supports two output modes:

```text
-f bin
-f obj
```

The mode should be explicit in project Makefiles.

## 2. Flat binary mode

```text
t32-as -f bin source.s -o source.bin
```

Flat binary mode preserves the current assembler behavior.

Properties:

- all symbols must resolve within the source file;
- `.org` defines the binary load address model used by the program;
- no symbol table is emitted;
- no relocation table is emitted;
- the output is directly loadable by `t32-run`.

Unresolved external symbols are errors in binary mode.

## 3. Relocatable object mode

```text
t32-as -f obj source.s -o source.o
```

Object mode emits T32OBJ v1.

Properties:

- section-relative addresses are emitted;
- unresolved declared external symbols are allowed;
- relocation records are emitted for link-time patches;
- local and global symbols are recorded;
- no final load address is assigned;
- `.org` is not permitted.

## 4. Default format

For compatibility, the default format remains:

```text
bin
```

However, maintained Makefiles should use an explicit format:

```make
t32-as -f bin program.s -o program.bin
t32-as -f obj strlen.s -o strlen.o
```

## 5. Required object-mode directives

### 5.1 `.section`

```asm
.section .text
.section .data
.section .bss
```

Object mode begins in `.text` when no `.section` directive appears.

Version 1 recognizes only:

```text
.text
.data
.bss
```

Unknown section names are rejected in version 1.

### 5.2 `.global`

```asm
.global strlen
```

The named symbol becomes globally visible.

The symbol may be defined before or after the directive.

A global symbol that remains undefined at end of assembly is an error unless
it is also declared with `.extern`.

### 5.3 `.extern`

```asm
.extern puts
```

The named symbol is an undefined global reference to be resolved by the
linker.

Defining an `.extern` symbol later in the same file is permitted; it then
becomes a normal global definition.

### 5.4 `.align`

```asm
.align 4
```

The current section offset advances to the next multiple of the requested
alignment.

Rules:

- alignment must be a positive power of two;
- `.text` and `.data` padding bytes are zero;
- `.bss` alignment increases section size without stored bytes.

### 5.5 Existing data directives

These remain valid:

```asm
.byte VALUE [, VALUE ...]
.word VALUE
```

A symbolic value in `.word` may generate `R_T32_ABS32`.

A symbolic value in `.byte` is allowed only when it resolves at assembly time
to a value from 0 through 255.

## 6. Label scope

Unmarked labels are local to the object file:

```asm
loop:
```

A label becomes globally visible only through `.global`.

T32OBJ v1 does not define source-level hidden, weak, or protected bindings.

## 7. `.equ`

`.equ` remains an assembly-time constant.

```asm
.equ STACK_TOP, 0x0000F000
```

Rules:

- `.equ` symbols are absolute;
- they do not require storage;
- they may be emitted as local absolute symbols;
- they may be made global with `.global`;
- an `.equ` expression must resolve during assembly.

Version 1 does not require general symbolic arithmetic.

## 8. `.org`

### Binary mode

Permitted according to the existing flat-binary rules.

### Object mode

Rejected:

```text
error: .org is not valid with -f obj
```

Section placement is a linker responsibility.

## 9. Undefined symbol rules

Object mode accepts an unresolved symbol only when declared with `.extern`.

Example:

```asm
.extern strlen
call strlen
```

An undeclared misspelled symbol remains an assembler error.

## 10. Recommended diagnostics

```text
error: duplicate symbol 'strlen'
error: undefined symbol 'strlne'; use .extern for external references
error: .org is not valid with -f obj
error: unknown section '.rodata'
error: alignment must be a positive power of two
error: symbol 'puts' declared extern and local
```
