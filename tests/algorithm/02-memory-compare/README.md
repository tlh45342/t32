# 02-memory-compare

Validates a fixed-length byte comparison routine analogous to the equality
portion of C `memcmp()`.

## Routine contract

```text
input
    r0 = left memory address
    r1 = right memory address
    r2 = byte count

output
    r3 = 0  equal
    r3 = 1  different
```

The initial T32 routine deliberately reports only equal/different. Ordering
semantics can be added later if the runtime needs a full three-way `memcmp`.

## Cases

```text
identical 16-byte blocks      -> equal
difference at byte 0          -> different
difference at byte 7          -> different
difference at byte 15         -> different
zero-length comparison        -> equal
```

The zero-length case uses two different buffers to prove that no byte is
examined when the requested length is zero.

## Run

```text
make test
```

Expected final line:

```text
PASS: memory-compare
```
