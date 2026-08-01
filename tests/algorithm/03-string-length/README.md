# 02-string-length

Validates a reusable null-terminated string-length algorithm.

## Test cases

- `""` -> 0
- `"A"` -> 1
- `"HELLO"` -> 5
- `"Hello World"` -> 11

The terminating zero byte is not included.

## Routine contract

```text
input:
    r0 = address of null-terminated string

output:
    r1 = length

clobbers:
    r0
    r2
```

This is provisional and is not yet the formal T32 ABI.

## Validation

The guest calls the same `strlen` routine for all four cases, compares each
result, stores the lengths in `r8` through `r11`, counts completed cases in
`r12`, and sets `r7` to zero only after all cases pass.

The host independently verifies those registers, the halted machine state,
and the HALT reason.

## Run

```text
make test
```

## Expected final state

```text
r7  = 0
r8  = 0
r9  = 1
r10 = 5
r11 = 11
r12 = 4
```

The source uses `.org 0x00001000`, matching the load address in `test.script`.
