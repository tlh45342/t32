# 12-memmove

Validates a libc-style `memmove` contract, including overlapping regions.

## Contract

```text
input:
    r0 = destination pointer
    r1 = source pointer
    r2 = byte count

output:
    r3 = original destination pointer
```

## Direction rule

The implementation copies backward only when the destination starts inside
the source range:

```text
source + 1 ... source + count - 1
```

All other cases copy forward. The overlap decision uses explicit pointer
equality checks and therefore does not require flag-consuming relational
branches.

## Cases

- zero-length operation;
- identical source and destination;
- non-overlapping copy;
- overlap with destination below source;
- overlap with destination inside source, requiring backward copy;
- one-byte copy.

The test also validates returned pointers, complete destination contents,
guard bytes, stack restoration, and guest PASS state.

Run:

```text
make test
```
