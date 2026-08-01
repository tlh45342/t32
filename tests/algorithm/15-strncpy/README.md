# 15-strncpy

Validates a libc-style `strncpy` contract.

## Contract

```text
input:
    r0 = destination pointer
    r1 = source pointer
    r2 = byte count

output:
    r3 = original destination pointer
```

## Required behavior

- copies at most `count` bytes;
- pads the destination with zeroes when the source terminates early;
- does not guarantee a terminator when source length is at least `count`;
- performs no memory access when `count == 0`.

## Cases

- zero-length copy;
- empty source with full zero padding;
- short source with partial zero padding;
- source length equal to count;
- source longer than count;
- one-byte copy.

The test also validates returned destination pointers, untouched bytes beyond
the requested range, guard bytes, stack restoration, and guest PASS state.

Run:

```text
make test
```
