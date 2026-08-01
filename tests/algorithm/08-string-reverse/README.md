# 08-string-reverse

Validates an in-place, zero-terminated string reversal routine for T32.

## Contract

```asm
; input:
;   r0 = pointer to a zero-terminated string
;
; output:
;   r0 = original string pointer
;
; behavior:
;   reverses the bytes before the terminating zero in place
;   leaves the terminating zero in place
```

The routine first measures the string, then swaps bytes from both ends. It
reduces the remaining byte count by two after each swap, so it does not require
division or signed relational branches.

## Cases

- `""` -> `""`
- `"A"` -> `"A"`
- `"AB"` -> `"BA"`
- `"T32"` -> `"23T"`
- `"Foundry"` -> `"yrdnuoF"`
- `"hello world"` -> `"dlrow olleh"`

The test also verifies:

- the guarded host-visible string remains within its buffer
- all six cases complete
- the stack pointer is restored
- the guest reports PASS
- the host independently observes the reversed bytes and intact guards

## Run

```text
make test
```
