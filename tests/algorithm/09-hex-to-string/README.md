# 09-hex-to-string

Converts an unsigned 32-bit value to exactly eight uppercase hexadecimal
digits followed by a zero terminator.

## Cases

```text
0x00000000 -> "00000000"
0x0000000A -> "0000000A"
0x00001234 -> "00001234"
0x89ABCDEF -> "89ABCDEF"
0xFFFFFFFF -> "FFFFFFFF"
```

The test also validates the returned destination pointer, guard bytes, stack
restoration, and all five guest-side comparison results.

Run:

```text
make test
```
