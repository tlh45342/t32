# T32 Historical Status Snapshot

**Snapshot:** August 2, 2026

> Historical snapshot. For current status see root `STATUS.md` and
> `docs/development/roadmap.md`.

## Summary

T32 has moved beyond a standalone virtual CPU project into a small, integrated software platform.

The repository now contains a working host toolchain, virtual machine, relocatable object and archive formats, ABI 0.1, static target runtime library, startup object, extensive validation, and a C compiler capable of producing and executing linked T32 programs.

The present development focus is `t32-cc`.

---

## Working End-to-End Path

The following path is currently validated:

```text
C source
  -> t32-cc
  -> T32 assembly
  -> t32-as
  -> relocatable T32OBJ object
  -> crt0.o + object + libt32.a
  -> t32-ld
  -> flat T32 binary
  -> t32-run
  -> ABI return value and HALT
```

The compiler has successfully generated programs that:

- return integer constants;
- use one initialized four-byte local integer;
- store the local on the T32 stack;
- load the local into `r0`;
- restore `r15`;
- return through ABI-compliant `RET`;
- re-enter `crt0`;
- halt with the expected result.

---

## Completed Components

### Architecture and VM

- Core ISA implementation
- Register file and stack behavior
- Arithmetic, logic, shifts, memory, branches, calls, and returns
- Virtual machine and monitor
- Host-native `libt32vm.a`
- Direct-mode and monitor-mode execution
- Dedicated VM smoke validation

### Toolchain

- `t32-as`
- `t32-nm`
- `t32-ld`
- `t32-ar`
- `t32-cc`
- T32OBJ relocatable object format
- T32AR static archive format
- Map-file generation
- Archive-member dependency resolution
- Root-level build, test, clean, and install workflow

### ABI and Runtime

- ABI 0.1 documentation
- Eight ABI conformance tests
- Static `libt32.a`
- Sixteen runtime archive members
- Runtime archive integration tests
- ABI-compliant `crt0.o`
- Installed target runtime layout:

```text
.local/lib/t32/
├── crt0.o
└── libt32.a
```

### Compiler

Current compiler version line:

```text
t32-cc 0.2.x
```

Current capabilities:

- `-S` assembly output
- `-c` relocatable object output
- default full linking
- `-o` output override
- quiet success
- `-v` phase/tool output
- constant integer returns
- one initialized local integer
- stack-based local storage
- semantic checks for undeclared and unsupported locals
- cleanup of partial outputs after failure
- positive and negative regression coverage

The current compiler test suite passes 48 cases.

---

## Current Root Workflow

From the repository root:

```text
make
make test
make install
make clean
```

The root Makefile builds and validates:

- all host toolchain components;
- `t32-run`;
- ABI tests;
- `libt32`;
- `crt0`.

During repository builds and tests, locally built tools are preferred over older installed copies.

---

## Current Installed Layout

```text
.local/
├── bin/
│   ├── t32-ar
│   ├── t32-as
│   ├── t32-cc
│   ├── t32-ld
│   ├── t32-nm
│   └── t32-run
├── include/
│   └── host VM interfaces
└── lib/
    ├── libt32vm.a
    └── t32/
        ├── crt0.o
        └── libt32.a
```

---

## Current Work

The next compiler milestone is assignment:

```c
int main(void)
{
    int x = 5;
    x = 7;
    return x;
}
```

This milestone introduces mutation of an existing symbol without introducing expression parsing or multiple local variables.

After assignment:

1. integer expressions;
2. comparisons;
3. `if` and `if/else`;
4. loops;
5. multiple locals and scopes;
6. functions and parameters;
7. pointers, arrays, and structures.

---

## Deferred Architecture Questions

The following are documented and intentionally deferred:

- packed `STATUS` representation;
- `MRS` and `MSR`;
- flag-consumption model;
- complete interrupt and task-switch context;
- upper-word multiply instructions;
- remainder instructions;
- shift-count edge behavior;
- unaligned-memory behavior;
- instruction-utilization measurements.

These are not treated as missing work that blocks the compiler. They are preserved as explicit questions awaiting evidence from compiler, runtime, firmware, and application workloads.

---

## Assessment

The project foundation is stable enough for compiler development.

The major subsystems no longer exist merely as isolated tools. They now form one validated chain:

```text
ISA -> VM -> assembler -> objects -> archives -> linker -> ABI -> runtime -> compiler
```

The immediate goal is to continue expanding the compiler without redesigning the ISA prematurely.
