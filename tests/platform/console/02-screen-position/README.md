# 02-screen-position

Validates random-access text video memory.

Expected display:

```text
HELLO

          WORLD
```

Locations:

- `HELLO`: row 0, column 0
- `WORLD`: row 2, column 10
- Video base: `0x90000000`
- WORLD offset: `(2 * 80) + 10 = 170 = 0xAA`
