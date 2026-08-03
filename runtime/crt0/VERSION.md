# crt0 Version History

## 0.0.1

- Added ABI 0.1 `_start`.
- Establishes the initial stack at `0x0000F000`.
- Calls external `main`.
- Preserves the `main` return value in `r0`.
- Halts after `main` returns.
- Added separate-compilation, link-map, and VM execution validation.
- Added user-local installation under `lib/t32/crt0.o`.
