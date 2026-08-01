# 11-memset

Validates a libc-style `memset` contract.

## Contract

```text
input:
    r0 = destination pointer
    r1 = fill value
    r2 = byte count

output:
    r3 = original destination pointer
```

Only the low eight bits of the fill value are stored.

## Cases

- zero-byte fill performs no memory access;
- one-byte fill;
- sixteen zero bytes;
- sixteen `0xFF` bytes;
- low byte of `0x12345678` produces `0x78`;
- thirty-two-byte fill using the low byte of `0xA5A5A53C`.

The test also validates returned pointers, source-value preservation, guard
bytes, stack restoration, and full-region contents.

Run:

```text
make test
```
