# libt32 Version History

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
