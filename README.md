# T32

*A small, documented, educational 32-bit computer architecture and software platform.*

------

# Overview

**T32** is an open-source project dedicated to demonstrating how an entire computer system is built.

Rather than beginning with an operating system or compiler, T32 starts at the foundation—the Instruction Set Architecture (ISA)—and builds upward one layer at a time. Every major component is documented, validated, and designed to be understandable.

The goal is not simply to build another virtual machine. The goal is to build a complete computing platform whose evolution can be followed from the first machine instruction through the assembler, linker, compiler, runtime library, firmware, and ultimately an operating environment.

Whether you're a student, educator, hobbyist, or simply curious about how computers work beneath the operating system, T32 is intended to be a platform that can be explored, studied, and extended.

------

# Project Status

The project currently includes:

- ✔ 32-bit Instruction Set Architecture (ISA)
- ✔ Virtual Machine
- ✔ Executable ISA Validation Suite
- ✔ ABI 0.1
- ✔ ABI Validation Suite
- ✔ Relocatable Object Format
- ✔ Static Archive Format
- ✔ T32 Assembler (`t32-as`)
- ✔ T32 Linker (`t32-ld`)
- ✔ T32 Symbol Inspector (`t32-nm`)
- ✔ T32 Archive Manager (`t32-ar`)
- ✔ Static Runtime Library (`libt32`)
- ✔ Algorithm Validation Suite
- ✔ Architecture Validation Suite
- ✔ Comprehensive Documentation
- ✔ Early-stage C Compiler (`t32-cc`)

Every significant subsystem is accompanied by automated validation tests. Documentation and executable behavior are developed together so that the implementation and its specification remain synchronized.

------

# Design Philosophy

T32 is built around a few guiding principles.

- Build from the foundation upward.
- Keep every subsystem understandable.
- Prefer clarity over cleverness.
- Document the engineering decisions.
- Validate behavior with executable tests.
- Make the journey as educational as the destination.

The objective is not merely to build software, but to understand *why* each layer exists and how those layers cooperate to form a complete computing system.

------

# Repository Layout

```text
docs/
    Architecture, ABI, runtime, and project documentation.

toolchain/
    Assembler, compiler, linker, archive manager, and related tools.

runtime/
    Runtime library, startup code, and supporting runtime components.

vm/
    Virtual machine implementations and execution support.

tests/
    ISA, ABI, algorithm, and platform validation.

validation/
    Higher-level integration and behavioral validation.

tools/
    Build and maintenance utilities.
```

------

# Roadmap

## Completed

- Instruction Set Architecture
- Virtual Machine
- Relocatable Object Format
- Static Archive Format
- Assembler
- Linker
- Archive Manager
- Symbol Inspector
- Runtime Library
- ABI 0.1
- ABI Validation Suite

## In Progress

- C Compiler (`t32-cc`)
- Runtime Startup (`crt0`)
- Runtime Refinement

## Planned

- BIOS
- Console Subsystem
- Block Storage
- T32 Filesystem (T32FS)
- System Monitor
- Operating Environment
- Expanded Standard Library
- Advanced Compiler Features

------

# Why T32 Exists

Most software projects begin near the top of the software stack and work downward.

T32 intentionally takes the opposite approach.

Beginning with the instruction set allows every subsequent layer—the assembler, linker, compiler, runtime library, firmware, operating system, and applications—to be developed on a well-understood foundation.

The project documents not only the finished result, but also the engineering decisions and development path that produced it.

The journey is considered just as valuable as the destination.

------

# Building

Typical development workflow:

```text
make
make test
make validation
```

Each subsystem may also be built and tested independently from its own directory.

------

# Contributing

Contributions, bug reports, documentation improvements, ideas, and thoughtful questions are always welcome.

One of the goals of T32 is to remain approachable. Curiosity is encouraged, and every contribution—large or small—helps improve the project.

------

# About the Author

**Thomas L. Hamilton**
Computer Engineer • Dog Nanny • Alternative LEGO® Enthusiast

I share my home with three dogs who take turns chaperoning me.

When I'm not writing software, you'll probably find me experimenting with LEGO® bricks, magnets, simple machines, and homemade generators—because engineering should always leave room for curiosity, exploration, and play.

------

# License

Copyright © Thomas L. Hamilton

Released under the **MIT License**.

See the accompanying **LICENSE** file for the complete license text.
