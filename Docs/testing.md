Describe the testing philosophy.

```
Each instruction requires

documentation

assembler support

execution support

regression tests

Tests initialize the machine through
test.script.

Assembly source should exercise only
the instruction under test.

The execution log becomes the source
of truth for verification.
```

Then document the directory layout:

```
tests/

    core-iset/

        01-movi
        02-mov
        03-add
```