# t32-cc Version History

## 0.2.0

- Added one initialized local `int` in `main`.
- Added compiler-local symbol name storage and identifier lookup.
- Assigned `int` a 4-byte stack slot.
- Added `STW` initialization and `LDW` return generation.
- Restores `r15` before `RET`.
- Added undeclared-variable, second-local, and missing-initializer negatives.
- Preserved Stage 2 `-S`, `-c`, link, quiet, verbose, and failure behavior.
