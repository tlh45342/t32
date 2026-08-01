# 03-string-compare

Validates equality comparison for two null-terminated strings.

## Routine contract

```text
input:
    r0 = pointer to string A
    r1 = pointer to string B

output:
    r2 = 0 when equal
    r2 = 1 when different

clobbers:
    r0
    r1
    r3
    r4
```

This is an equality primitive, not yet a libc-style lexicographical `strcmp`.

## Test cases

| String A | String B | Expected |
|---|---|---:|
| `""` | `""` | 0 |
| `"A"` | `"A"` | 0 |
| `"HELLO"` | `"HELLO"` | 0 |
| `"HELLO"` | `"HELLX"` | 1 |
| `"ABC"` | `"ABCD"` | 1 |
| `"ABCD"` | `"ABC"` | 1 |

The guest stores results in `r8` through `r13`, counts completed cases in
`r14`, and sets `r7` to zero only after all six cases pass. The host verifies
those results, normal HALT, and restoration of the `r15` stack pointer.

## Run

```text
make test
```

## Expected final state

```text
r7  = 0
r8  = 0
r9  = 0
r10 = 0
r11 = 1
r12 = 1
r13 = 1
r14 = 6
r15 = 0x0000F000
```
