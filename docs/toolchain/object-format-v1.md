# T32OBJ v1 Object Format

## 1. Purpose

T32OBJ is the relocatable object format shared by:

```text
t32-as -f obj
t32-nm
t32-ld
t32-ar
```

A T32OBJ file contains section data, symbols, relocations, and names needed to
link separately assembled T32 modules.

T32OBJ v1 is little-endian and uses fixed-width integer fields.

## 2. File layout

A T32OBJ v1 file is arranged as:

```text
+-----------------------------+
| Header                      |
+-----------------------------+
| Section table               |
+-----------------------------+
| Symbol table                |
+-----------------------------+
| Relocation table            |
+-----------------------------+
| String table                |
+-----------------------------+
| Section contents            |
+-----------------------------+
```

Offsets stored in the file are absolute byte offsets from the beginning of
the file.

## 3. Header

The header is 48 bytes.

```c
struct t32obj_header_v1 {
    uint8_t  magic[8];
    uint16_t version_major;
    uint16_t version_minor;
    uint32_t flags;
    uint32_t section_count;
    uint32_t symbol_count;
    uint32_t relocation_count;
    uint32_t section_table_offset;
    uint32_t symbol_table_offset;
    uint32_t relocation_table_offset;
    uint32_t string_table_offset;
    uint32_t string_table_size;
};
```

### 3.1 Magic

```text
54 33 32 4F 42 4A 00 00
 T  3  2  O  B  J \0 \0
```

### 3.2 Version

```text
version_major = 1
version_minor = 0
```

A reader must reject an unsupported major version.

A reader may accept a newer minor version only when all unknown flags and
entry types can be safely ignored.

### 3.3 Flags

Version 1 defines:

```text
0x00000001  T32OBJ_FLAG_LITTLE_ENDIAN
```

All other bits must be zero when written and must be rejected when read.

## 4. Section table

Each section entry is 32 bytes.

```c
struct t32obj_section_v1 {
    uint32_t name_offset;
    uint32_t type;
    uint32_t flags;
    uint32_t alignment;
    uint32_t size;
    uint32_t data_offset;
    uint32_t reserved0;
    uint32_t reserved1;
};
```

### 4.1 Section types

```text
0  T32OBJ_SECTION_NULL
1  T32OBJ_SECTION_PROGBITS
2  T32OBJ_SECTION_NOBITS
```

`PROGBITS` has bytes stored in the file.

`NOBITS` reserves memory but stores no section bytes. It is intended for
`.bss`.

### 4.2 Section flags

```text
0x00000001  ALLOC
0x00000002  EXEC
0x00000004  WRITE
```

Recommended mappings:

```text
.text  PROGBITS  ALLOC | EXEC
.data  PROGBITS  ALLOC | WRITE
.bss   NOBITS    ALLOC | WRITE
```

### 4.3 Alignment

Alignment is expressed in bytes and must be a power of two.

Valid examples:

```text
1 2 4 8 16
```

Version 1 writers should normally use:

```text
.text = 4
.data = 1 or 4
.bss  = 4
```

### 4.4 Section zero

Section index zero is reserved as the null section.

It must contain all-zero fields.

Real sections begin at index one.

## 5. Symbol table

Each symbol entry is 24 bytes.

```c
struct t32obj_symbol_v1 {
    uint32_t name_offset;
    uint32_t section_index;
    uint32_t value;
    uint32_t size;
    uint8_t  binding;
    uint8_t  type;
    uint16_t flags;
    uint32_t reserved;
};
```

### 5.1 Section index values

```text
0x00000000  undefined symbol
0xFFFFFFFF  absolute symbol
otherwise   index into the section table
```

### 5.2 Symbol value

For a defined section symbol:

```text
value = byte offset from the beginning of its section
```

For an absolute symbol:

```text
value = absolute numeric value
```

For an undefined symbol:

```text
value = 0
```

### 5.3 Bindings

```text
0  LOCAL
1  GLOBAL
```

Local symbols are visible only inside the object file.

Global symbols may satisfy references from other object files.

### 5.4 Types

```text
0  NOTYPE
1  FUNCTION
2  OBJECT
3  SECTION
```

Type information is descriptive in version 1. The linker does not require
function and object symbols to be treated differently.

### 5.5 Symbol zero

Symbol index zero is reserved as the null symbol and must contain zeroes.

## 6. Relocation table

Each relocation entry is 20 bytes.

```c
struct t32obj_relocation_v1 {
    uint32_t section_index;
    uint32_t offset;
    uint32_t symbol_index;
    uint32_t type;
    int32_t  addend;
};
```

The relocation describes a patch within the named section.

```text
section_index = section containing the patch
offset        = byte offset within that section
symbol_index  = referenced symbol
type          = relocation operation
addend        = signed constant added to the symbol value
```

## 7. String table

The string table stores section and symbol names.

Rules:

- Byte zero must be `0`.
- Every name is UTF-8 text terminated by `0`.
- `name_offset` is a byte offset into the string table.
- Offset zero means no name.
- Writers may deduplicate identical names.

Tool-generated symbol names should remain ASCII where practical.

## 8. Section contents

`PROGBITS` section bytes appear at each section's `data_offset`.

`NOBITS` sections have:

```text
data_offset = 0
```

The file does not store bytes for `NOBITS`.

## 9. Validation requirements

A reader must reject a file when:

- the magic is incorrect;
- the major version is unsupported;
- an unknown mandatory flag is set;
- a table extends beyond the file;
- a string offset is outside the string table;
- a string is not terminated within the string table;
- a section index is invalid;
- a relocation offset is outside its target section;
- an alignment is zero or not a power of two;
- a `PROGBITS` section extends beyond the file;
- reserved fields are nonzero.

## 10. File extension

Relocatable T32 object files use:

```text
.o
```

The extension does not replace validation of the magic and version.
