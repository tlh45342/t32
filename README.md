# T32

*A small, documented, educational 32-bit computer architecture and software platform.*

---

## Overview

**T32** is an open-source project for exploring how a complete computer system is built from the instruction set upward.

Rather than beginning with an operating system or compiler, T32 starts at the foundation—the Instruction Set Architecture (ISA)—and builds upward one validated layer at a time.

The project includes a virtual machine, assembler, object format, linker, archive manager, symbol inspector, ABI, runtime library, startup environment, and a growing C compiler.

The goal is not simply to build another virtual machine. The goal is to create a complete, understandable computing platform whose development can be followed from the first machine instruction through the assembler, linker, compiler, runtime, firmware, devices, and eventually an operating environment.

T32 is intended for students, educators, hobbyists, and anyone curious about how computers work beneath the operating system.

---

## What T32 Is

T32 is:

- A deterministic 32-bit Instruction Set Architecture
- A virtual machine and execution environment
- A compiler target
- A systems experimentation platform
- A learning and exploration tool
- A foundation for exploring compilers, runtimes, firmware, devices, and operating systems

T32 is intentionally simple, but not simplistic.

It is large enough to support meaningful software development and systems experimentation while remaining small enough for the architecture, toolchain, runtime, and execution model to be understood by a single developer.

## What T32 Is Not

T32 is not:

- A replacement for x86
- A replacement for ARM
- A replacement for RISC-V
- A commercial CPU design
- A production virtualization platform

The project exists to explore concepts, implementations, and engineering tradeoffs in a manageable environment.

---

## Current Project Status

T32 currently includes:

- 32-bit Instruction Set Architecture
- T32 virtual machine and monitor (`t32-run`)
- Executable core-ISA validation
- ABI 0.1 and ABI validation
- Relocatable T32OBJ object format
- T32AR static archive format
- Assembler (`t32-as`)
- Linker (`t32-ld`)
- Symbol inspector (`t32-nm`)
- Archive manager (`t32-ar`)
- C compiler (`t32-cc`)
- Static target runtime library (`libt32.a`)
- ABI-compliant startup object (`crt0.o`)
- Algorithm and architecture validation suites
- Root-level build, test, clean, and install orchestration
- Architecture, ABI, runtime, toolchain, and project documentation

The compiler supports an end-to-end source-to-execution path:

```text
C source
  -> T32 assembly
  -> relocatable object
  -> crt0.o + object + libt32.a
  -> linked flat binary
  -> t32-run
```

`t32-cc` is being developed incrementally. Each new language feature is validated through the complete compiler, assembler, linker, runtime, and virtual-machine execution path.

---

## Design Philosophy

T32 is built around a small set of guiding principles:

- Build from the foundation upward.
- Keep every subsystem understandable.
- Prefer clarity over cleverness.
- Document decisions and unresolved questions.
- Validate behavior with executable tests.
- Favor explicit operands and visible data flow.
- Let measurements and compiler experience guide ISA evolution.
- Make the journey as educational as the destination.

The objective is not merely to produce working software, but to understand why each layer exists and how the layers cooperate to form a complete computing system.

---

## Repository Layout

```text
docs/
    Architecture, ABI, runtime, toolchain, development, and project documentation.

toolchain/
    t32-as, t32-cc, t32-ld, t32-ar, and t32-nm.

runtime/
    libt32, crt0, and supporting runtime components.

vm/
    t32-run and execution support.

tests/
    Core ISA, ABI, algorithm, architecture, platform, and system validation.

validation/
    Higher-level calling, control-flow, stack, and integration validation.

tools/
    Repository build and maintenance utilities.
```

---

## Building

The root repository provides a common development workflow:

```text
make
make test
make install
make clean
```

The root build prefers tools produced in the current checkout over older copies installed in the user's `PATH`.

Each subsystem may also be built, tested, installed, and cleaned independently.

### Default Installation

On Windows:

```text
%USERPROFILE%\.local\bin
%USERPROFILE%\.local\lib
%USERPROFILE%\.local\lib\t32
%USERPROFILE%\.local\include
```

On Linux and macOS:

```text
$HOME/.local
```

---

## Installed Layout

A typical installation resembles:

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
│   └── host-side VM interfaces
└── lib/
    ├── libt32vm.a
    └── t32/
        ├── crt0.o
        └── libt32.a
```

`libt32vm.a` contains host-native virtual-machine code.

`lib/t32/libt32.a` contains T32 target machine code.

These libraries serve different purposes and are intentionally separate.

---

## Current Development Focus

Current development is focused on expanding the C compiler and validating the architecture through increasingly capable software.

Compiler development proceeds in deliberately small stages:

```text
source language feature
        ↓
compiler
        ↓
assembler
        ↓
relocatable object
        ↓
linker + runtime
        ↓
T32 executable
        ↓
t32-run
        ↓
validated behavior
```

This approach allows compiler development to exercise the ISA, ABI, assembler, linker, runtime, and virtual machine together.

Architectural questions discovered during this work are documented and resolved deliberately rather than hidden behind implementation assumptions.

---

## Planned Platform Work

Planned work includes:

- Continued C compiler development
- BIOS or firmware startup
- Memory-mapped text console
- Timer and RTC
- Block storage and disk ABI
- Keyboard and network devices
- T32FS
- System monitor
- Program loader and operating environment
- Expanded C runtime and standard-library support

The exact order may evolve as compiler and systems experiments expose new requirements.

---

## Documentation

Detailed documentation is maintained under [`docs/`](docs/).

The documentation covers areas including:

- Programmer model
- Instruction set
- Instruction encoding
- Memory map
- ABI
- Runtime
- Toolchain
- Testing
- Implementation status
- Design decisions
- Roadmap

Documentation and executable behavior are developed together. When implementation and specification differ, the discrepancy should be identified and resolved deliberately rather than silently redefining the architecture.

---

## Contributing

Contributions, bug reports, documentation improvements, ideas, and thoughtful questions are welcome.

T32 is intended to remain approachable. Curiosity is encouraged, and every contribution—large or small—can improve the project.

---

## About the Author

**Thomas L. Hamilton**  
Computer Engineer, Dog Nanny, Alternative Lego Enthusiast

Three dogs take turns chaperoning the author while he experiments with computers, magnets, Lego generators, and other interesting systems.

---

## License

Copyright © Thomas L. Hamilton

Released under the **MIT License**.

See the accompanying `LICENSE` file for the complete license text.