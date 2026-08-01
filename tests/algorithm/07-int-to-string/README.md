# 07 — Integer to String

Validates an unsigned integer-to-decimal-string routine for T32.

## Contract

```asm
; input:
;   r0 = destination buffer
;   r1 = unsigned value, 0..99999
;
; output:
;   r2 = original destination buffer
```

The routine writes canonical, zero-terminated ASCII decimal text:

- no leading zeroes
- zero is written as `"0"`
- internal zeroes are preserved
- the terminating zero byte is always written

## Cases

| Input | Expected output |
|---:|---|
| 0 | `"0"` |
| 7 | `"7"` |
| 42 | `"42"` |
| 100 | `"100"` |
| 1002 | `"1002"` |
| 12345 | `"12345"` |

The final destination is guarded and independently examined by the Python
validator.

## Implementation note

This first implementation does not require division or remainder instructions.
It increments a five-digit decimal odometer once for each input unit, then emits
the resulting digits. This is intentionally simple and architecture-neutral,
although not intended as the final high-performance conversion routine.

## Run

```text
make test
```

## Image layout

The test uses one `.org` directive at `0x00001000`. A leading `JMP` skips embedded validation data, placing the host-inspected guarded destination at `0x00001008`.
