# 06-string-to-int

Validates a small T32 routine that converts a zero-terminated ASCII decimal string into an unsigned integer.

## Contract

```asm
; input
;   r0 = source string pointer
;
; output
;   r2 = converted unsigned integer
;
; clobbers
;   r0-r6
```

This first version deliberately accepts only valid unsigned decimal input. An empty string returns zero, and leading zeroes are accepted. Sign handling, overflow reporting, whitespace, and invalid characters are left for a later extended conversion test.

The conversion uses:

```text
result = result * 10 + digit
```

Multiplication by ten is implemented with repeated `ADD` operations so the test does not depend on a multiply instruction.

## Cases

1. `""` → `0`
2. `"0"` → `0`
3. `"7"` → `7`
4. `"42"` → `42`
5. `"12345"` → `12345`
6. `"00042"` → `42`

The harness also verifies the completed-case count, restored stack pointer, HALT state, and final PASS register.

## Run

```text
make test
```
