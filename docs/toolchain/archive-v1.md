# t32-ar Version 1

## 1. Purpose

`t32-ar` packages T32OBJ files into a static archive.

The primary initial archive is:

```text
libt32.a
```

## 2. Basic commands

```text
t32-ar rcs libt32.a file.o ...
t32-ar t   libt32.a
t32-ar x   libt32.a
t32-ar d   libt32.a member.o
```

Initial operation letters:

```text
r  replace or add members
c  create archive
s  build symbol index
t  list members
x  extract members
d  delete members
```

## 3. Archive format

Version 1 should use a simple T32-specific archive container rather than
pretending to be Unix `ar`.

Recommended magic:

```text
T32AR\0\0\0
```

Each archive member stores:

- member name;
- byte offset;
- byte size;
- member data;
- exported-global-symbol index entries.

The exact archive binary layout should be specified when `t32-ar`
implementation begins.

## 4. Symbol index

The archive symbol index maps:

```text
global symbol name -> archive member
```

Only defined global symbols are indexed.

Local and undefined symbols are not indexed.

## 5. Linker extraction rule

`t32-ld` extracts an archive member when:

```text
the member defines a symbol currently unresolved by the link
```

After extraction, newly introduced undefined symbols may cause additional
members to be extracted.

A member is extracted at most once.

## 6. Determinism

Archives should be reproducible.

Version 1 should avoid storing host timestamps, user IDs, or platform-specific
permissions unless normalized.

Given the same members in the same order, the archive bytes should be
identical.

## 7. Member naming

Member names are UTF-8 strings.

For portability, build systems should use simple ASCII basenames:

```text
strlen.o
strcmp.o
memset.o
```

Duplicate member names are replaced by `r`.

## 8. Initial libt32 command

```text
t32-ar rcs libt32.a \
    memset.o memcpy.o memmove.o memcmp.o memchr.o \
    strlen.o strcpy.o strncpy.o strcmp.o strncmp.o strchr.o strstr.o
```
