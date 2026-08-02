# libt32 0.0.3

`libt32` is now a real T32 static library.

## Build

```text
make            build build/libt32.a
make objects    build individual T32OBJ files
make archive    build build/libt32.a
make inspect    inspect representative objects and list archive members
make test       link and execute programs through libt32.a
make clean
```

## Tool requirements

```text
t32-as >= 0.0.9
t32-nm >= 0.0.1
t32-ld >= 0.0.2
t32-ar >= 0.0.1
t32-run
```

## Validation

The integration suite links through `libt32.a` rather than naming routine
objects directly. It verifies selective archive extraction with:

- a one-function `strlen` program;
- a multi-function program using `strlen` and `strcmp`;
- map checks proving unrelated members are not extracted;
- VM execution and register validation.
