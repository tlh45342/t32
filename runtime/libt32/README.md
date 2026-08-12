# libt32 0.0.7

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

## I/O milestone: putchar

libt32 0.0.5 adds `putchar` using the T32 ABI 0.1 register convention:

```text
r0 = character byte
call putchar
r0 = character byte
```

The initial implementation writes to the memory-mapped text framebuffer and
maintains a library-owned cursor. It provides the first target-runtime service
called directly from compiler-generated C code.

## I/O milestone: puts

libt32 0.0.6 adds `puts`:

```text
r0 = pointer to zero-terminated byte string
call puts
r0 = 0
```

`puts` preserves the callee-saved register set, writes each byte through
`putchar`, and appends a newline. `putchar` now treats byte `10` as newline and
advances its library-owned framebuffer cursor to column zero of the next row.


## I/O milestone: bounded console and scrolling

libt32 0.0.7 makes the text-console cursor safe for sustained interactive use.

`putchar` now guarantees that its library-owned cursor remains inside the
80x25 framebuffer (`0x90000000` through `0x900007CF`). It supports:

- ordinary character output with natural 80-column wrapping;
- LF (`\n`) to column zero of the next row;
- CR (`\r`) to column zero of the current row;
- scrolling when output advances beyond the final row.

Scrolling copies rows 1..24 to rows 0..23, clears row 24 with spaces, and leaves
the cursor at row 24 column 0. The implementation is entirely guest-side T32
code and keeps console state in guest memory.

The regression suite includes a console-scroll program that deliberately emits
enough lines to cross the bottom of the framebuffer and verifies that execution
halts normally without an out-of-range MMIO write.
