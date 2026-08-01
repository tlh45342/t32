# 17-strncmp

Validates a libc-style `strncmp` contract.

## Contract

```text
input:
    r0 = pointer to left string
    r1 = pointer to right string
    r2 = maximum byte count

output:
    r3 = first unequal unsigned-byte difference
         zero when equal within count bytes
```

The comparison stops at the first mismatch, equal terminators, or exactly
`count` bytes.

## Cases

- zero-count comparison;
- empty strings;
- identical strings;
- mismatch within the limit;
- mismatch beyond the limit;
- left string terminates within the limit;
- count ends before a length difference;
- unsigned high-bit byte comparison.

Run:

```text
make test
```
