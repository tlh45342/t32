# t32-cc 0.25.2

String-literal terminator regression fix.

- Guarantees that every C string literal emits its required trailing NUL byte.
- Fixes literals whose visible length is an exact multiple of the assembler emitter's eight-byte chunk width.
- Adds focused 7/8/9 and 15/16/17 character boundary regressions.
- Preserves the 0.25.1 struct dot/arrow fixes.
