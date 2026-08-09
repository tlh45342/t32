# T32 C Stage 3 0.0.4

`NEXT.BIN` is now a mixed assembly/C program that uses a real target runtime
string service.

The build chain is:

```text
stage3.s                         machine-facing startup
main.c --t32-cc--> build/main.s
       --t32-as--> build/main.o
libt32.a                         putchar + puts
               \              /
                t32-ld -Ttext 0x00020000
                         |
                      NEXT.BIN
```

The C source is intentionally small:

```c
int main(void)
{
    int rc;
    rc = puts("Hello from C via puts()");
    return 42 + rc;
}
```

This milestone adds the first compiler-supported string literal. A string
literal is emitted as a zero-terminated byte sequence in `.data`; its address
is passed in `r0` to `puts` using the existing T32 function-call ABI.

`puts` lives in `libt32`, walks the target string through ordinary memory,
calls `putchar` for each byte, appends a newline, and returns zero.

The language step is deliberately narrow. T32 C still does not expose general
pointer declarations, pointer arithmetic, dereference syntax, arrays, or
`char *`. Internally, however, this is already a real pointer value: the C
call passes the linked address of the string object to a separately linked
runtime routine.

## Build and test

```text
make clean
make test
```

From the repository root:

```text
make test-firmware
```

The full test proves:

```text
BIOS -> BOOT.BIN -> NEXT.BIN -> C main()
                              -> puts(string address)
                              -> libt32
                              -> framebuffer
```

The expected guest text includes:

```text
Hello from C via puts()
```
