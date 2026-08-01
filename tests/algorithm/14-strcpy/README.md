# 14-strcpy

Validates a libc-style `strcpy` contract.

- Copies through and including the terminating zero byte.
- Returns the original destination pointer.
- Tests empty, one-character, short, word, and phrase inputs.
- Verifies guard bytes and stack restoration.

Run with `make test`.
