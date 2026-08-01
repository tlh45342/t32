# T32 Testing Guide

## Philosophy

The test suite is intended to become an ISA and runtime conformance suite, not merely a collection of examples.

A completed instruction requires:

1. documentation;
2. assembler support;
3. runtime decode and execution support;
4. exact encoding validation;
5. normal-result validation;
6. boundary and exceptional cases;
7. flag validation where applicable;
8. alias and source-preservation regression tests.

## Directory layers

```text
tests/
    core-iset/       instruction-level validation
    algorithm/       multi-instruction software routines
    architecture/    reset, exception, interrupt, and ABI behavior
    platform/        console, timer, RTC, disk, network, MMIO
    system/          boot and integrated machine workflows
```

## Local test contents

A test directory normally contains:

```text
Makefile
source assembly
run_tests.py
README.md
```

Generated binaries and logs may be retained or ignored according to repository policy, but must always be reproducible.

## What tests should check

As applicable:

- successful assembly;
- exact bytes;
- image load address;
- halt or fault reason;
- instruction count;
- destination result;
- source preservation;
- complete flags;
- memory contents and guards;
- pointer advancement;
- stack restoration and contents;
- taken and not-taken branches;
- host-observed side effects;
- exceptional behavior.

## Smoke versus conformance

A smoke test proves one ordinary path can assemble and execute.

A conformance test covers normal, boundary, alias, flag, and exceptional behavior.

## Pass criteria

A directory passes only when every build and validation step succeeds and `run_tests.py` exits with status zero.
