# T32 Testing Guide

## Purpose

The T32 test suite is intended to become an ISA conformance suite, not merely a collection of examples.

A complete instruction is expected to have:

1. assembler support;
2. runtime decode and execution support;
3. documented semantics;
4. exact encoding validation;
5. normal-result validation;
6. relevant boundary and exceptional cases;
7. flag validation where applicable;
8. regression coverage for aliasing and source preservation.

## Directory layout

Current instruction tests live under:

```text
tests/core-iset/
```

Directory names combine the decimal opcode and mnemonic:

```text
00-halt
24-add
32-and
51-ret
```

A smoke-test directory generally contains:

```text
Makefile
<instruction>.s
<instruction>.bin
<instruction>.log
run_tests.py
test.script
```

Expanded tests contain several assembly cases and generated binaries/logs:

```text
add_normal.s
add_zero.s
add_carry.s
add_overflow.s
add_negative.s
add_alias.s
run_tests.py
Makefile
```

## Running tests

From the project root:

```text
make test
```

A single instruction directory can be run with:

```text
make -C tests/core-iset/24-add test
```

The local Makefile assembles each `.s` source and then runs `run_tests.py`.

## Runtime test sequence

Smoke tests use a `test.script` similar to:

```text
logfile add.log
version
reset
load add.bin 0x1000
set pc 0x1000
set run steps 4
run
regs
status
logfile off
```

Expanded tests construct equivalent scripts dynamically for each case.

## What a test should check

Every case should check all applicable items:

- expected binary exists;
- exact assembler encoding;
- image loaded at the intended address;
- final machine state;
- halt or fault reason when relevant;
- exact instruction count;
- destination register value;
- source registers remain unchanged unless aliasing is intentional;
- complete applicable flag set;
- memory bytes or words changed as expected;
- stack pointer and stack contents;
- branch taken/not-taken behavior;
- exceptional behavior.

## Smoke tests versus expanded tests

### Smoke test

A smoke test answers:

> Can the assembler encode this instruction and can the runtime execute one ordinary example?

It is necessary, but it is not complete validation.

### Expanded conformance test

An expanded test answers:

> Does the instruction behave correctly across ordinary, boundary, flag, alias, and exceptional cases?

Current expanded suites exist for ADD, ADDI, SUB, SUBI, CMP, and CMPI.

## Case-design standard

For register-result instructions, consider:

```text
ordinary result
zero result
negative result
maximum/minimum values
carry or borrow
signed overflow
identity values (0 or 1)
destination aliases source A
destination aliases source B
source registers preserved
```

For immediate instructions, additionally consider:

```text
immediate zero
smallest accepted immediate
largest accepted immediate
sign/width interpretation
```

For memory instructions, consider:

```text
byte/half/word boundaries
known byte order
aligned access
misaligned access
lowest/highest legal addresses
unmapped access
source preservation
```

For branches and calls, consider:

```text
taken
not taken
forward target
backward target
branch over HALT
loop behavior
return address
stack restoration
```

## Logging

Each instruction test currently produces local runtime logs such as:

```text
add_normal.log
and.log
```

For a whole-suite console log on PowerShell:

```powershell
make test 2>&1 | Tee-Object -FilePath t32-test.log
```

A future top-level test runner should:

- stream output to the terminal;
- create a timestamped suite log;
- update a predictable `latest` log;
- count directories and validation cases;
- preserve a nonzero exit status on failure;
- print a final conformance summary.

## Generated files

`.bin` and `.log` files are generated test artifacts. Whether they remain committed should be a deliberate repository policy. If excluded, Makefiles must always be able to recreate them.

## Pass criteria

A test directory passes only when:

- every assembly step succeeds;
- every case executes successfully;
- every expected value is found;
- no unexpected runner failure occurs;
- `run_tests.py` exits with status 0.

The top-level `make test` passes only when every selected test directory passes.
