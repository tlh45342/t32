# T32 Virtual Computer

*A developmental virtual computer architecture for exploring instruction set design, compiler construction, operating systems, and virtualization.*

**Author:** Thomas Hamilton

---

# Overview

T32 is a developmental 32-bit virtual computer architecture created to explore the design and implementation of complete computing systems from first principles.

Unlike commercial processor architectures that have evolved over decades, T32 intentionally begins with a minimal instruction set and expands incrementally. Every instruction, subsystem, and virtual device is introduced only after it has been specified, implemented, validated, and documented.

The objective is not to build the fastest processor or compete with existing architectures. Instead, T32 provides an environment where architectural decisions remain visible, understandable, and open to discussion.

T32 serves as both a practical software development platform and an educational vehicle for studying computer architecture, compiler construction, firmware, operating systems, and virtualization.

---

# Development Status

**T32 is an active developmental project.**

The instruction set, ABI, virtual devices, firmware interfaces, executable format, compiler, operating system, and development tools continue to evolve.

Backward compatibility between early versions is not guaranteed. Refinement through experimentation is considered an essential part of the project's development process.

---

# Project Philosophy

The project is built around several guiding principles:

* Keep the architecture understandable.
* Introduce complexity only when justified.
* Validate every feature before depending upon it.
* Document engineering decisions.
* Build reusable components with clean interfaces.
* Learn by constructing complete systems rather than isolated examples.

Many architectural decisions intentionally remain open until practical implementation demonstrates whether additional complexity is warranted.

For example:

* Should conditional branches depend upon processor flags?
* Should branches inspect registers directly?
* Which instructions belong in hardware?
* Which features should instead be implemented by the compiler?
* When does minimalism become unnecessary inconvenience?

These questions are considered part of the project itself.

---

# Development Methodology

Every major feature follows the same engineering process:

```text
Specification
      ↓
Implementation
      ↓
Validation
      ↓
Documentation
      ↓
Reuse
```

Validation programs are intentionally small and focused. Once validated, they become reusable building blocks for firmware, operating systems, libraries, and applications.

---

# Goals

The long-term goals of T32 include:

* Design a clean virtual instruction set.
* Develop a complete virtual machine implementation.
* Build an assembler, linker, compiler, and debugger.
* Explore compiler construction through incremental bootstrapping.
* Develop firmware and monitor software.
* Build a small operating system.
* Construct reusable virtual devices.
* Provide an educational platform for systems programming.
* Serve as the reference architecture for developing the Foundry ecosystem.

---

# Non-Goals

T32 is **not** intended to:

* compete with x86 or ARM processors
* maximize execution performance
* emulate existing hardware
* preserve historical compatibility
* replace commercial virtualization platforms

Its purpose is education, experimentation, and incremental engineering.

---

# Why T32?

Modern processors contain decades of accumulated complexity.

T32 intentionally starts with very little.

A smaller instruction set makes it practical to explore topics that are normally hidden within mature architectures, including:

* instruction selection
* addressing modes
* calling conventions
* compiler code generation
* executable formats
* interrupt handling
* firmware design
* operating system construction

Rather than inheriting architectural decisions, T32 encourages understanding **why** those decisions exist.

---

# Relationship to Foundry

Although T32 is a complete architecture in its own right, it also serves as the initial reference architecture for the Foundry ecosystem.

**Foundry is a developmental virtual machine hosting ecosystem.**

Foundry is intentionally architecture-independent.

Its purpose is to host, manage, and orchestrate virtual machines regardless of the guest processor architecture.

Current and future execution environments may include:

* T32
* x64-vm
* armvm
* additional architectures

Only execution nodes are architecture-specific.

Infrastructure services remain reusable across all supported virtual machine implementations.

---

# Architecture Overview

The ecosystem separates architecture-specific software from shared infrastructure.

```text
                 Guest Software
                        │
                Firmware / OS
                        │
               Architecture Library
                        │
                Execution Node
────────────────────────────────────────
               Foundry Platform
────────────────────────────────────────
 VM Management
 Virtual Storage
 Virtual Networking
 Console Services
 REST APIs
 Scheduling
 Administration
```

This separation allows infrastructure services to evolve independently from processor architectures.

---

# Ecosystem

## Architecture-Specific Components

### libt32

Reference implementation of the T32 virtual machine.

Provides:

* CPU execution
* memory management
* interrupt processing
* virtual devices
* execution state

---

### t32-as

Reference assembler for the T32 instruction set.

Converts assembly language into executable binary images.

---

### t32-run

Interactive execution and debugging environment.

Features include:

* program loading
* execution
* debugging
* register inspection
* memory inspection
* console display
* validation support

---

### t32-node

Architecture-specific execution node for Foundry.

Hosts T32 virtual machines and provides the interface between libt32 and the Foundry hosting platform.

Future execution nodes will provide equivalent services for additional processor architectures.

---

# Foundry Infrastructure

The following services are intentionally architecture-independent.

---

## Foundry

Virtual machine hosting ecosystem.

Provides:

* VM lifecycle management
* orchestration
* scheduling
* REST APIs
* image management
* infrastructure services

---

## vmctl

Command-line management client.

Provides administrative access to Foundry services.

---

## Web Management Interface

Browser-based administration interface for Foundry.

Provides graphical management of:

* virtual machines
* execution nodes
* storage
* networking
* infrastructure

---

## VConsole

Architecture-independent virtual console service.

Provides console connectivity for all supported virtual machine architectures.

---

## VDisk

Architecture-independent virtual storage service.

Provides virtual block storage independently of the guest processor architecture.

Current implementations are expected to utilize VVdisk from r32lib as the underlying storage engine.

---

## Switchyard

Architecture-independent virtual networking environment.

Provides:

* virtual Ethernet segments
* virtual switching
* network attachment
* packet forwarding
* bridge support
* network isolation
* future routing capabilities

---

# Engineering Tools

These tools assist in building, provisioning, and deploying complete virtual systems.

---

## Guppy

Architecture-independent disk image and filesystem management tool.

Guppy prepares and manages virtual machine storage without requiring knowledge of the guest processor architecture.

Primary capabilities include:

* create virtual disk images
* partition disks
* create filesystems
* copy files into and out of virtual disk images
* build bootable system images
* automate deployment through scripting
* support repeatable provisioning workflows

Guppy is intended to be fully scriptable and suitable for scheduled automation, continuous integration, and repeatable system deployment.

---

# Compiler Development

Compiler development is expected to proceed incrementally.

Rather than attempting to build a complete compiler immediately, increasingly capable stages will be developed, including:

* constant expressions
* arithmetic
* variables
* conditional execution
* loops
* procedures
* pointers
* structures
* runtime libraries

The long-term objective is to demonstrate the traditional compiler bootstrap process by constructing increasingly capable development tools using earlier generations of those same tools.

---

# Validation

Validation is considered a first-class engineering activity.

Every instruction, subsystem, and virtual device is accompanied by focused validation programs.

Examples include:

* Hello World
* String Copy
* Screen Position
* Character Overwrite
* Screen Clear

These programs become reusable examples and eventually evolve into firmware, operating-system components, and application software.

---

# Long-Term Vision

The long-term objective is to develop an understandable and reusable virtual computing platform from the instruction set upward.

The complete ecosystem is expected to include:

* instruction set architecture
* assembler
* linker
* compiler
* executable format
* debugger
* virtual machine
* firmware
* operating system
* virtual devices
* deployment tools
* virtual machine hosting
* distributed management

The emphasis throughout the project remains on clarity, modularity, validation, and documentation.

---

# License

MIT License

Copyright (c) Thomas Hamilton

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

---

# Author

Thomas Hamilton

Computer Scientist

T32 is developed as an exploration of computer architecture, compiler construction, operating systems, virtualization, and distributed systems. The project emphasizes understanding complete systems through careful engineering, validation, and documentation while providing the reference architecture for the broader Foundry virtual machine hosting ecosystem.
