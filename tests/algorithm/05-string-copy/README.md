# 05-string-copy

Validates a zero-terminated T32 string-copy routine using register-based `JZ/JNZ`.

## Run

```text
make test
```

The guest executes five copy cases and validates return pointers, contents, guards, case count, and stack restoration. The Python harness independently examines the final guarded destination at `0x00001323`.
