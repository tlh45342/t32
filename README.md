# T32

*A small, documented, educational 32-bit computer architecture and software platform.*

---

## Overview

**T32** is an open-source project dedicated to demonstrating how an entire computer system is built.

Rather than beginning with an operating system or compiler, T32 starts at the foundation—the Instruction Set Architecture—and builds upward one validated layer at a time. The project now includes a virtual machine, assembler, object format, linker, archive manager, symbol inspector, ABI, runtime library, startup object, and a growing C compiler.

The goal is not merely to build another virtual machine. The goal is to build a complete computing platform whose evolution can be followed from the first machine instruction through the assembler, linker, compiler, runtime, firmware, devices, and eventually an operating environment.

T32 is intended for students, educators, hobbyists, and anyone curious about how computers work beneath the operating system.

---

## Current Project Status

T32 currently includes:

- 32-bit Instruction Set Architecture
- T32 virtual machine and monitor (`t32-run`)
- Executable core-ISA validation
- ABI 0.1 and an eight-part ABI validation suite
- Relocatable T32OBJ object format
- T32AR static archive format
- Assembler (`t32-as`)
- Linker (`t32-ld`)
- Symbol inspector (`t32-nm`)
- Archive manager (`t32-ar`)
- C compiler driver (`t32-cc`)
- Static target runtime library (`libt32.a`)
- ABI-compliant startup object (`crt0.o`)
- Algorithm and architecture validation suites
- Root-level build, test, clean, and install orchestration
- Architecture, ABI, runtime, toolchain, and project documentation

The compiler currently supports the complete source-to-execution path:

```text
C source
  -> T32 assembly
  -> relocatable object
  -> crt0.o + object + libt32.a
  -> linked flat binary
  -> t32-run
```

`t32-cc` currently supports constant returns and one initialized stack-based local integer. Assignment, expressions, comparisons, and control flow are the next compiler milestones.

---

## Design Philosophy

T32 is built around a small set of guiding principles:

- Build from the foundation upward.
- Keep every subsystem understandable.
- Prefer clarity over cleverness.
- Document both decisions and unresolved questions.
- Validate behavior with executable tests.
- Favor explicit operands and visible data flow.
- Let measurements and compiler experience guide ISA evolution.
- Make the journey as educational as the destination.

The objective is not merely to produce working software, but to understand why each layer exists and how the layers cooperate to form a complete system.

---

## Repository Layout

```text
docs/
    Architecture, ABI, runtime, toolchain, development, and project documents.

toolchain/
    t32-as, t32-cc, t32-ld, t32-ar, and t32-nm.

runtime/
    libt32, crt0, and future runtime components.

vm/
    t32-run and future execution-node components.

tests/
    Core ISA, ABI, algorithms, architecture, platform, and system validation.

validation/
    Higher-level calling, control-flow, and stack integration tests.

tools/
    Repository build and maintenance utilities.
```

---

## Building

The root repository exposes a common workflow:

```text
make
make test
make install
make clean
```

The root build prefers tools produced in the current checkout over older copies installed in the user PATH.

On Windows, installation currently uses:

```text
%USERPROFILE%\.local\bin
%USERPROFILE%\.local\lib
%USERPROFILE%\.local\lib\t32
%USERPROFILE%\.local\include
```

On Linux and macOS, the corresponding default prefix is:

```text
$HOME/.local
```

Each subsystem may also be built, tested, installed, and cleaned independently.

---

## Installed Layout

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

`libt32vm.a` contains host-native VM code. `lib/t32/libt32.a` contains T32 target machine code. They are intentionally separate.

---

## Current Development Focus

The present focus is the incremental construction of `t32-cc`.

Compiler milestones are intentionally narrow:

```text
0.1.x  compiler driver and ABI-compliant object generation
0.2.x  initialized local integer and symbol lookup
0.3.x  assignment and mutation
next    expressions, comparisons, selection, iteration, and functions
```

Each language feature is validated through the full compiler, assembler, linker, runtime, and VM path.

Open ISA questions—such as a packed STATUS register, `MRS/MSR`, upper-word multiplication, remainder instructions, shift semantics, and alignment—are documented and deferred until compiler and system workloads provide evidence.

---

## Planned Platform Work

After the compiler foundation is stronger, planned work includes:

- BIOS or firmware startup
- memory-mapped text console and KVM integration
- timer and RTC
- block storage and disk ABI
- keyboard and network devices
- T32FS
- system monitor
- loader and operating environment
- expanded C runtime and standard-library support

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
