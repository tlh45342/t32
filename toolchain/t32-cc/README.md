# t32-cc 0.4.0 Addition Patch

Changed-files-only update for `toolchain/t32-cc`.

## New language forms

```c
return 5 + 3;
return x + 3;
return 3 + x;
return x + x;
x = x + 1;
x = 4 + 5;
```

Expressions currently permit exactly one binary `+`. Each operand may be an
integer literal (including a negative literal) or the one declared local
integer.

## Deliberately deferred

- chained addition
- parentheses
- subtraction
- multiplication and division
- multiple locals
- precedence

## Test

From `O:\Foundry\t32\toolchain\t32-cc`:

```bat
make clean
make
make test
make install
```

The suite also corrects the stale version label and now expects `0.4.0`.
