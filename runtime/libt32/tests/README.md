# libt32 ABI 0.1 integration tests

The suite links only through `libt32.a` and validates:

- scalar return values in `r0`;
- multi-routine archive extraction;
- `r8-r14` preservation by a routine that uses preserved registers internally;
- stack restoration and clean HALT.
