# 16-strcmp

Validates a libc-style `strcmp`.

- returns zero when strings are equal;
- returns a negative value when the left string sorts first;
- returns a positive value when the left string sorts later;
- returns the exact unsigned-byte difference at the first mismatch.

Cases include empty strings, equal strings, earlier/later mismatches,
shorter/longer strings, case differences, and high-bit byte values.

Run:

```text
make test
```
