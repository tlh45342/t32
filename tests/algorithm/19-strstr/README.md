# 19-strstr

Validates a libc-style `strstr` contract.

This corrected version keeps the routine's internal search state in `r0-r6`.
Registers `r8-r13` are reserved by the validation program for saved test
results and are not clobbered by `strstr`.

Run:

```text
make test
```
