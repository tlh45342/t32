# T32

*A small architecture for exploring big ideas.*

T32 is a deterministic 32-bit instruction set architecture (ISA), toolchain,
runtime, firmware stack, and virtual machine designed for systems exploration.

The project starts at the machine instruction and deliberately builds upward:
assembler, object format, linker, ABI, runtime library, C compiler, virtual
hardware, firmware, boot media, and execution environment.

T32 is intentionally simple, but not simplistic. It is large enough to support
meaningful systems work while remaining small enough for one person to follow
the complete machine from source code to execution.

## T32 Running C

T32 can now compile, link, and execute C code through its own toolchain and
runtime on the T32 virtual machine.

<p align="center">
  <img src="images/t32-runx-hello-c.png"
       alt="T32 running a compiled C program in t32-runx"
       width="760">
</p>

The program shown above is deliberately unremarkable:

```c
int main(void)
{
    int rc;

    rc = puts("Hello Thomas");
    return 7;
}
```

What is interesting is the path that produced it:

```text
hello.c
   |
   |  t32-cc
   v
T32 assembly
   |
   |  t32-as
   v
T32 relocatable object
   |
   |  t32-ld + installed libt32.a
   v
hello.bin
   |
   |  t32-runx / libt32vm
   v
T32 virtual machine
   |
   |  T32 display MMIO
   v
Hello Thomas
```

This is not host-side printing disguised as target execution. `t32-cc` emits
T32 code, `t32-as` creates T32 objects, `t32-ld` resolves the program against
the T32 target runtime, and `libt32vm` executes the resulting T32 instructions.

The current `libt32` console path is intentionally suitable for bare-metal
programs and writes through the T32 machine interface. As supervisor and
operating-system layers develop, higher-level services can move behind the T32
SVC ABI while bare-metal access remains available to firmware and standalone
software.

## Current Milestone

The project has crossed from an ISA/VM experiment into a small but coherent
software platform. The currently demonstrated stack includes:

- a documented 32-bit T32 ISA;
- executable ISA and ABI validation;
- `t32-as`, the T32 assembler;
- a relocatable object format;
- `t32-ld`, the T32 linker;
- `t32-ar`, the archive manager;
- `t32-nm`, the symbol inspector;
- `t32-cc`, the early-stage T32 C compiler;
- `libt32`, the target static runtime library;
- `libt32vm`, the reusable virtual-machine core;
- `t32-run`, the command-line VM/monitor;
- `t32-runx`, the Windows graphical developer runner;
- memory-mapped text display and keyboard/platform facilities;
- a synchronous virtual block-device interface;
- `t32-disk` and the bootstrap T32D disk-image format;
- BIOS Bootinfo handoff and BIOS disk-read service;
- `BOOT.BIN` second-stage loading;
- compiler-built `NEXT.BIN` third-stage execution.

The firmware validation now exercises a complete path in which BIOS loads
`BOOT.BIN` from T32D media, BOOT uses the BIOS disk service to load `NEXT.BIN`,
and the third stage can contain C code built by the T32 compiler and linked
against `libt32`.

## Why T32 Exists

Modern architectures are extraordinarily capable, but their complexity can
obscure the systems concepts being studied.

T32 exists to provide a smaller environment for exploring subjects such as:

- instruction-set design;
- assembler, linker, and compiler construction;
- ABI and runtime design;
- firmware and boot processes;
- operating systems and supervisor interfaces;
- virtual devices and storage;
- debugging and execution environments;
- virtualization, migration, and orchestration.

The objective is not to compete with x86, ARM, or RISC-V. The objective is to
make the entire stack understandable enough that architectural decisions can be
implemented, tested, discussed, and changed.

## Design Philosophy

T32 is built from first principles and follows a few practical rules:

- build from the foundation upward;
- keep subsystem boundaries visible;
- prefer clarity over cleverness;
- document contracts and engineering decisions;
- validate behavior with executable tests;
- keep host presentation separate from guest-visible machine contracts;
- allow simple implementations now without preventing cleaner abstractions
  later.

The journey is part of the project: understanding *why* each layer exists is as
important as producing the final layer.

## Architecture at a Glance

```text
C source
   |
t32-cc
   |
T32 assembly
   |
t32-as
   |
T32 objects --------+
                    |
libt32.a ------------+--> t32-ld --> T32 program
                                      |
                                      v
                              +----------------+
                              |    libt32vm    |
                              | CPU / RAM/MMIO |
                              +----------------+
                                ^            ^
                                |            |
                           t32-run        t32-runx
```

The firmware boot path adds another set of deliberately visible layers:

```text
T32 VM
  |
BIOS
  |
  +-- publishes Bootinfo
  +-- publishes BIOS services
  |
T32D disk
  |
BOOT.BIN
  |
NEXT.BIN
  |
C / runtime code
```

`t32-run` and `t32-runx` are intentionally single-vCPU developer/reference
hosts. Larger VM/node orchestration belongs above the reusable `libt32vm`
machine core.

## Repository Layout

```text
docs/
    Architecture, ABI, device, runtime, and project documentation.

firmware/
    BIOS, BOOT.BIN, and later-stage firmware/bootstrap programs.

images/
    Project-level screenshots and README artwork.

runtime/
    T32 target runtime libraries and startup support.

toolchain/
    Assembler, C compiler, linker, archive manager, symbol tools, and related
    target-development utilities.

tools/
    Host-side utilities such as t32-disk.

vm/
    libt32vm plus command-line and graphical VM hosts.

tests/
    Project-level tests.

validation/
    ISA, ABI, integration, and behavioral validation.
```

## Installed Development Environment

T32 tools are intended to be usable outside the source repository. A normal
per-user installation follows the conventional local prefix:

```text
~/.local/
    bin/
        t32-as
        t32-ar
        t32-cc
        t32-ld
        t32-nm
        t32-run
        ...

    lib/
        t32/
            crt0.o
            libt32.a
```

On Windows, `~` corresponds to `%USERPROFILE%`.

A standalone project therefore does not need to know where the T32 repository
is located. It can invoke the installed tools and link against the installed
target runtime.

## Building and Validation

The root Makefile is the entry point for coordinated project builds and tests.
Individual components also retain their own Makefiles so they can be developed
and validated independently.

Common project-level validation includes:

```text
make test-vm
make test-firmware
```

The validation philosophy is straightforward: specifications, implementation,
and executable tests should evolve together. A feature is considerably more
interesting once the repository can demonstrate it repeatedly.

## Firmware and Boot Direction

The current bootstrap path is deliberately small:

```text
BIOS
  -> discovers/validates T32D media
  -> publishes Bootinfo and BIOS services
  -> loads BOOT.BIN

BOOT.BIN
  -> validates Bootinfo
  -> uses BIOS disk service
  -> loads NEXT.BIN

NEXT.BIN
  -> receives the machine handoff
  -> can execute compiler-built C/runtime code
```

T32D is a bootstrap disk-image format, not a claim that the long-term
filesystem problem is solved. The block-device contract, disk layout,
filesystem, and boot policy remain separate concerns.

## Runtime Direction

The current target runtime supports the bare-metal environment being developed
today. That makes direct machine services useful and testable now.

The longer-term layering can evolve toward:

```text
application
    |
libt32 / language runtime
    |
SVC / system ABI
    |
supervisor or operating environment
    |
device drivers
    |
T32 MMIO
```

Firmware and standalone bare-metal programs can continue to use the machine
interfaces directly. Introducing an SVC layer later does not invalidate the
bare-metal path; it adds another execution environment above it.

## T32 and Foundry

T32 is the first architecture used within the broader Foundry work.

Foundry explores higher-level virtualization and orchestration concepts such as
machine lifecycle, console services, storage abstractions, networking, state
management, and migration. T32 provides a compact machine on which those ideas
can be developed and observed.

T32 nevertheless remains a standalone project. Its ISA, toolchain, runtime,
firmware, VM, documentation, and tests are useful independently of Foundry.

## Roadmap

Near-term work continues upward from the now-demonstrated C execution and boot
stack:

- refine `t32-cc` language coverage and compiler-driver behavior;
- mature `libt32` and its installed SDK layout;
- formalize SVC/supervisor-facing runtime services;
- continue firmware and boot-service development;
- expand platform devices, including timer/RTC and interrupt delivery;
- evolve storage beyond the bootstrap T32D format;
- develop a small operating/supervisor environment;
- keep `t32-runx` useful as a compact graphical development/reference host;
- connect the reusable VM core to the larger Foundry node/service architecture.

## Project Status

T32 is under active development. Interfaces that have been demonstrated and
validated are increasingly stable, while compiler, firmware, runtime, and
higher-level system services are still evolving.

For detailed implementation status, see `STATUS.md`. Architecture and contract
documentation lives under `docs/`.

---

**T32 is intentionally small enough to understand, but complete enough to make
the layers above the ISA real.**
