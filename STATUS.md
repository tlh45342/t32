# T32 Status

## Current State

T32 has progressed from instruction-set validation into a small, working
bare-metal software platform.

The current repository demonstrates the complete path from C source through the
T32 compiler, assembler, linker, installed target runtime, virtual machine, and
memory-mapped display. Firmware can also boot a compiler-built third stage from
T32D media through BIOS and BOOT.BIN.

The ISA, ABI, toolchain, runtime, VM core, display, keyboard, platform-control,
block-device, bootstrap disk format, and early firmware chain now have executable
validation.

## Completed

### Architecture and validation

- Core 32-bit T32 ISA implementation
- Executable instruction validation suite
- ABI 0.1 and ABI validation
- Relocatable object format
- Static archive format
- Algorithm and platform validation
- Memory-mapped 80x25 text display

### Toolchain

- `t32-as` assembler
- `t32-ld` linker
- `t32-ar` archive manager
- `t32-nm` symbol inspector
- Early-stage `t32-cc` C compiler
- Standalone C programs can be compiled, assembled, linked, and executed
- Compiler-generated calls can resolve target routines from installed `libt32.a`

### Runtime

- `libt32` target static runtime library
- Installed target layout under `.local/lib/t32`
- `putchar`
- `puts`
- string, memory, and conversion routines
- C return values follow the current T32 calling convention
- Bare-metal console output through T32 display MMIO

The current direct-MMIO runtime path is intentional for bare-metal programs.
Higher-level application services can later move behind the T32 SVC ABI without
removing direct hardware access for firmware and standalone software.

### VM core

- Reusable `libt32vm`
- `t32-run` command-line VM/monitor
- `t32-runx` Windows graphical developer/reference host
- Single-vCPU execution model
- Register, PC, flags, instruction-count, machine-state, and halt-reason
  inspection
- Guest-requested POWER_OFF and RESET platform controls
- Configured RAM-size platform query
- Guest-visible RTC with validated ID, status, and UTC epoch registers

### Keyboard

The first T32 keyboard path is implemented and validated.

The current path is:

```text
host keyboard
    |
t32-runx
    |
libt32vm keyboard queue
    |
T32 keyboard MMIO
    |
guest
```

Validated behavior includes:

- host ASCII input accepted by `libt32vm`;
- queued-byte/pending count tracking;
- keyboard STATUS register is guest-readable;
- STATUS reports data-ready while input is queued;
- keyboard DATA register is guest-readable;
- DATA returns the queued ASCII byte;
- reading DATA consumes that byte;
- `t32-runx` forwards interactive Windows character input into `libt32vm`.

This is deliberately a minimal text/ASCII keyboard contract. Scan codes,
modifier-state modeling, interrupts, and richer input semantics remain future
work.

### Virtual disk and T32D

- Synchronous 512-byte sector-oriented virtual block device
- Host-file disk backend
- Guest READ and WRITE through disk MMIO
- Disk geometry exposed to the guest
- `t32-disk` host utility
- T32D v0.1 bootstrap disk-image format
- Fixed bootstrap directory
- File put/get/list/info support
- Scriptable `t32-disk` command engine
- `BOOT.BIN` and `NEXT.BIN` installation into T32D images

T32D is a bootstrap media format, not the final T32 filesystem.

### RTC

- RTC MMIO is implemented and guest-readable.
- Device ID reports `T3R1`.
- STATUS exposes valid time.
- EPOCH exposes host UTC epoch seconds.
- RTC registers are read-only.
- Standalone guest RTC validation passes.
- Stage3 `time` is validated through the full BIOS -> BOOT -> NEXT chain.

RTC is wall-clock time. A separate monotonic timer/counter is planned for
elapsed-time measurement and benchmarking.

### Firmware and boot

- T32 BIOS
- Disk-presence and T32D validation
- BIOS Bootinfo handoff
- Bootinfo v0.2 / 72-byte structure
- RAM size and sector-size publication
- BIOS service v0.1 publication
- BIOS `disk_read` service
- BIOS loading and transfer to `BOOT.BIN`
- `BOOT.BIN` validation of Bootinfo
- BOOT use of BIOS disk service
- BOOT loading and transfer to `NEXT.BIN`
- Compiler-built C third stage
- Third-stage Bootinfo handoff validation
- C code executing through the complete BIOS -> BOOT -> NEXT chain
- Interactive Stage3 C monitor 0.0.13
- Stage3 `help`, `version`, `bootinfo`, `mem`, `time`, and `halt`
- Scripted and edited monitor command parsing

### t32-runx

`t32-runx` is the Windows single-vCPU graphical developer/reference host.

Current demonstrated features include:

- 80x25 T32 text display;
- interactive keyboard input;
- firmware selection;
- Disk 0 attach/detach;
- Start, Stop, and Reset lifecycle controls;
- guest POWER_OFF behavior;
- direct flat-binary program loading at `0x00020000`;
- modeless read-only CPU Stats window;
- register and PC display;
- machine state, instruction count, flags, and halt reason;
- Help/About version identification
- Embedded default BIOS for disk-and-go startup
- External BIOS override for firmware development/testing
- Corrected interactive monitor prompt/input behavior

A standalone C program using `puts("Hello Thomas")` has been compiled with the
T32 toolchain, linked against the installed `libt32.a`, loaded directly by
`t32-runx`, executed on `libt32vm`, and displayed through T32 video MMIO. Its
`return 7` result is observable in `r0`.

## In Progress

- Incremental expansion of `t32-cc`
- Runtime/library refinement
- Firmware service refinement
- Platform-contract documentation
- Continued integration and regression validation
- Separating bare-metal runtime services from future supervisor/SVC services

## Next Platform Work

1. Stage3 `run` and fixed-address external flat-binary loading.
2. External HELLO/SORT program built outside the repository.
3. Clean application return to Stage3.
4. Small application-facing SVC ABI.
5. Monotonic timer/counter.
6. Deterministic SORT benchmark.
7. Small versioned executable-image contract.
8. Additional standalone validation/sample programs.
9. Storage evolution beyond T32D, with ext2 as a target.

Continued `t32-cc`/`libt32` work should also add normal target headers such as
`stdio.h`, `stdlib.h`, `string.h`, `stdint.h`, and `stddef.h`, allowing programs
to inherit prototypes through conventional `#include` use.

## Architectural Boundaries

`t32-run` and `t32-runx` are intentionally single-vCPU reference/developer
hosts. Multi-vCPU/SMP and larger machine orchestration belong to the later
VM/node architecture.

The intended long-term service layering remains:

```text
T32 guest
    |
guest-visible display / keyboard / disk / future NIC contracts
    |
libt32vm
    |
t32-node
    |
VCONSOLE / VDISK / SWITCHYARD
    |
Foundry
```

Windows-specific presentation belongs in `t32-runx`; guest-visible machine
contracts belong in `libt32vm` and the T32 architecture.

## Still Planned

- Monotonic timer/counter device
- Interrupt-driven device operation
- Mature SVC/supervisor ABI implementation
- T32 filesystem beyond the bootstrap T32D format
- System monitor / operating environment
- Expanded C language support
- Expanded standard/runtime library
- Networking
- `t32-node` and Foundry-facing VM service integration
