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
- Reusable host-native VM core (`libt32vm`)
- T32 command-line/headless monitor (`t32-run`)
- Windows interactive developer host (`t32-runx`)
- Executable core-ISA validation
- ABI 0.1 and an eight-part ABI validation suite
- Relocatable T32OBJ object format
- T32AR static archive format
- Assembler (`t32-as`)
- Linker (`t32-ld`)
- Symbol inspector (`t32-nm`)
- Archive manager (`t32-ar`)
- C compiler (`t32-cc` 0.16.0, through function-maturity/recursion groundwork)
- Static T32 target runtime library (`libt32.a`)
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

`t32-cc` 0.16.0 now supports precedence-aware integer arithmetic, signed
comparisons, `if/else`, `while`, `for`, `break`, `continue`, multiple locals,
expression initializers, ordinary uninitialized locals, multiple integer
functions and parameters, nested calls, calls inside expressions, early returns,
and recursion groundwork. The Stage 17 compiler suite is validated at
**1047/1047 checks passing**.

Pointers, arrays/strings, richer integer types, aggregates, globals/static
storage, broader preprocessing, and a freestanding C library remain major
upcoming compiler/runtime milestones.

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
    libt32vm, t32-run, t32-runx, t32-node, and future execution hosts.

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


## Root Build

The repository root is the build conductor for the complete T32 development
platform.

```text
make
make test
make install
make clean
```

At repository root, `make` builds the toolchain, VM components, and firmware.
`make install` intentionally means **install the complete T32 platform**.
Individual subprojects retain their own narrower `make install`.

Useful focused targets are:

```text
make toolchain
make vm
make firmware
make test-toolchain
make test-vm
make install-toolchain
make install-vm
make install-firmware
```

Host executables install under `~/.local/bin`, the VM library/headers under
`~/.local/lib` and `~/.local/include`, and firmware under:

```text
~/.local/share/t32/firmware/bios.bin
```

On Windows, `~` is `%USERPROFILE%`. `t32-runx` remains Windows-only and is
skipped on other hosts.

---

## Where We Are So Far

T32 has crossed from an ISA/toolchain experiment into a small interactive
computer platform.

The current stack is:

```text
T32 guest / BIOS
      |
      | instructions + MMIO
      v
   libt32vm
      |
      +-- 80x25 display
      +-- keyboard FIFO
      +-- virtual disk
      +-- platform lifecycle control
      |
      +------------------+
      |                  |
   t32-run            t32-runx
 CLI/headless        Windows GUI
```

The first BIOS now runs as T32 guest code, writes its banner through display
MMIO, polls the keyboard device, echoes host keystrokes, detects the virtual
disk, and can request machine power-off through platform-control MMIO.

That means the current interactive round trip is real:

```text
Windows key
  -> t32-runx
  -> libt32vm keyboard FIFO
  -> keyboard MMIO
  -> BIOS / T32 CPU
  -> display MMIO
  -> t32-runx window
```

The lifecycle path is likewise guest-driven:

```text
BIOS writes POWER_OFF
  -> platform-control MMIO
  -> libt32vm records request
  -> host powers off VM
```

`t32-runx` remains deliberately Windows-only and single-vCPU. Its purpose is
to provide a fast local reference host while the portable/enterprise path
continues toward `t32-node`, VCONSOLE, VDISK, SWITCHYARD, and Foundry.

### Current MMIO summary

```text
0x90000000  text display    80x25, one ASCII byte per cell
0x90001000  disk            synchronous 512-byte sector device
0x90002000  keyboard        polling ASCII FIFO
0x90003000  currently unused/reserved
0x90004000  platform        guest POWER_OFF / RESET requests
```

The VM core is now being separated into the independent `vm/libt32vm` project so
`t32-run`, `t32-runx`, and eventually `t32-node` all consume one canonical CPU
and device implementation.

### Compiler waypoint

`t32-cc` is at **0.16.0 / Stage 17**. It has moved well beyond the bootstrap
compiler and now exercises meaningful stack frames, ABI calls, nested function
evaluation, structured control flow, and recursion. The next high-value compiler
frontier is systems-oriented C: pointers first, followed by arrays/strings,
type maturity, aggregates, globals/static storage, and the freestanding library
needed to write progressively more firmware/kernel code in C.

### Near-term platform waypoint

The immediate high-value platform work is expected to include:

```text
independent libt32vm ownership
t32-runx lifecycle/UI refinement
general device-service loop
timer
RTC
IRQ delivery
asynchronous disk completion
boot-sector / BOOTBIN loading
T32 disk layout + filesystem work
```

Networking is intentionally later, but the architecture is preserving a path
toward a virtual NIC, SWITCHYARD, and eventually network boot.

---

## Planned Platform Work

Platform work is now advancing in parallel with the compiler. Display,
keyboard, disk, a first BIOS, and guest lifecycle MMIO are already present.

Near/future work includes:

- general VM device-service loop
- timer, RTC, IRQ delivery, and interrupt-driven device completion
- BOOTBIN and boot-device policy
- T32 disk layout / initial filesystem work, followed quickly by EXT2 study
- `t32-disk` reference image utility
- expanded freestanding C runtime and local library
- pointer/array/type expansion in `t32-cc`
- primitive kernel and operating environment
- virtual NIC, SWITCHYARD, and network boot experiments
- `t32-node` integration with VCONSOLE/VDISK/Foundry

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
