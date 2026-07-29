# 03-overwrite-cell

Validates that an existing video-memory cell can be changed in place.

Program sequence:

```text
HELLO
```

Then row 0, column 1 is overwritten:

```text
E -> A
```

Expected display:

```text
HALLO
```

Video addresses:

- Screen base: `0x90000000`
- Overwritten cell: `0x90000001`
