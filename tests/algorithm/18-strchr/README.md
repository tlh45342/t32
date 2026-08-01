# 18-strchr

Validates a libc-style `strchr` contract.

## Contract

```text
input:
    r0 = pointer to zero-terminated string
    r1 = search value

output:
    r2 = pointer to first matching character
         pointer to the terminator when searching for zero
         zero when no match is found
```

Only the low eight bits of the search value are used.

## Cases

- first character;
- middle character;
- duplicate returns first occurrence;
- absent character;
- search for the terminator;
- empty string searched for the terminator;
- high-bit byte using low-byte-only search semantics.

Run:

```text
make test
```
