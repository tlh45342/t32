# libt32 integration tests

`00-strlen-linked` proves separate compilation and linking:

```text
main.s -> main.o
strlen.s -> strlen.o
main.o + strlen.o -> strlen-linked.bin
```

The test inspects `strlen.o` with `t32-nm`, links with `t32-ld`, executes with
`t32-run`, and expects `r1 = 5`.
