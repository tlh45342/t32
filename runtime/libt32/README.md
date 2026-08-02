# libt32 0.0.4

`libt32` is the ABI 0.1-conforming T32 static runtime library.

## Build

```text
make
make objects
make archive
make inspect
make test
make clean
```

## ABI 0.1

- arguments: `r0-r3`;
- primary return: `r0`;
- secondary status when documented: `r1`;
- caller-saved: `r0-r7`;
- callee-saved: `r8-r14`;
- stack pointer: `r15`, restored on return.

The test suite links through `libt32.a`, checks selective extraction, executes
linked programs, validates `r0` returns, and checks preserved registers.
