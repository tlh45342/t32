# 10-string-to-hex

Validates conversion of exactly eight hexadecimal digits to an unsigned
32-bit value.

## Contract

```text
input:
    r0 = pointer to eight hexadecimal digits followed by zero

output:
    r1 = parsed unsigned 32-bit value
    r2 = 0 on success, 1 on invalid input
```

Uppercase and lowercase digits are accepted. Invalid characters, short input,
and extra input are rejected. On failure, `r1` is zero.

## Cases

```text
"00000000" -> 0x00000000
"0000000A" -> 0x0000000A
"00001234" -> 0x00001234
"89ABCDEF" -> 0x89ABCDEF
"deadbeef" -> 0xDEADBEEF
"FFFFFFFF" -> 0xFFFFFFFF
"1234567G" -> invalid
"1234"     -> invalid
```

## Design note

The routine uses parallel character and nibble-value tables. This avoids
requiring relational branch instructions and fits T32's register-tested
`JZ/JNZ` model.

Run:

```text
make test
```
