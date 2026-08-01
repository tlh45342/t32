# 13-memchr

Validates a libc-style `memchr` contract.

## Contract

```text
input:
    r0 = memory pointer
    r1 = search value
    r2 = byte count

output:
    r3 = pointer to first matching byte
         zero when no match is found
```

Only the low eight bits of the search value are compared.

## Cases

- match at the first byte;
- match in the middle;
- duplicate value returns the first occurrence;
- restricted range finds the last matching byte;
- absent value returns zero;
- zero-length search returns zero without reading memory.

The test also verifies that source memory and guard bytes remain unchanged,
the stack is restored, and all six cases complete.

Run:

```text
make test
```
