# libt32 0.0.1

`libt32` is the reusable source-library home for routines proven by the T32
algorithm validation suite.

## Current status

This version is **source-only**. It does not pretend that T32 object files,
linking, or static archives exist yet.

Today:

```text
make
```

runs a manifest and source-layout check.

Future:

```text
t32-as -f obj ...
t32-nm ...
t32-ld ...
t32-ar ...
```

will allow the same sources to become `libt32.a`.

## Layout

```text
libt32/
├── include/
├── src/
│   ├── memory/
│   ├── string/
│   └── convert/
├── docs/
├── tests/
├── tools/
├── build/
├── Makefile
└── manifest.json
```

## Included routines

Memory:

```text
memset memcpy memmove memcmp memchr
```

String:

```text
strlen strcpy strncpy strcmp strncmp strchr strstr strrev
```

Conversion:

```text
atoi hex_to_string string_to_hex
```

## Important distinction

```text
tests/algorithm/
    validates independent executable examples

libt32/
    owns the reusable routine implementations
```

The next phase should make the algorithm tests consume these implementations
through relocatable object files rather than carrying private copies.
