# libt32 Version History

## 0.0.4

- Refactored all 16 public routines to T32 ABI 0.1.
- Standardized primary return values in `r0`.
- Standardized optional secondary status in `r1`.
- Added explicit preservation of `r8-r14` where required.
- Updated public include contracts and ABI documentation.
- Updated archive integration tests for ABI return values.
- Added direct callee-saved preservation validation for `string_to_hex`.

## 0.0.3

- Added real `libt32.a` creation with `t32-ar`.
- Made `make` build the static archive by default.
- Added archive listing to `make inspect`.
- Updated linked tests to consume `libt32.a` through `t32-ld 0.0.2`.
- Added selective extraction checks for used and unused members.
- Added a multi-routine archive integration test using `strlen` and `strcmp`.

## 0.0.2

- Added relocatable-object builds for all 16 routines.
- Added linked `strlen.o` integration validation.
