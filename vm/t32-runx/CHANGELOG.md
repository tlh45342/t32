# t32-runx Changelog

## 0.0.6 - direct program loading and CPU stats

- add `File -> Load Program...` for flat binaries at `0x00020000`;
- loading a direct program sets PC but does not auto-start the VM;
- retain the existing BIOS + Disk 0 boot workflow as a separate mode;
- add modeless `View -> Stats...` window with r0-r15, PC, state, instruction
  count, flags, and halt reason;
- refresh stats while running and on machine lifecycle changes.

# CHANGELOG

All notable changes to **t32-run** are documented in this file.

---

## Unreleased

**Last Updated:** 2026-07-27

### Changed

* Standardized the runtime build system.
* Changed the default compiler from `cc` to `gcc`.
* Cleaned the repository structure.
* Established the external `t32` repository as the canonical ISA validation suite.
* Updated project documentation.

### Fixed

* Corrected the runtime build configuration.
* Removed assembler-specific artifacts from the runtime repository.

---

## Version 0.0.6

**Release Date:** 2026-08-08

### Added

- Added a synchronous 512-byte sector virtual disk controller at MMIO base `0x90001000`.
- Added host-file disk attach/detach APIs to `libt32vm`.
- Added `disk create`, `disk attach`, `disk info`, and `disk detach` monitor commands.
- Added a 512-byte PIO sector buffer at `0x90001100`.
- Added guest-driven READ and WRITE validation against a real host disk image.

### Changed

- Extended memory routing so guest `LDB/LDH/LDW` and `STB/STH/STW` can reach the disk MMIO range.
- Machine reset now resets disk controller state while leaving attached media present.

---

## Version 0.0.5

## Release Date: 2026-07-29

### Added

- Added an 80x25 memory-backed text display at `0x90000000`.
- Added `display` and `display raw` CLI commands.
- Added display dirty-state tracking for future live viewers.

### Changed

- Machine reset now clears video memory to spaces.
- Memory reads and writes now recognize the video-memory range.

## Version 0.0.4

**Release Date:** 2026-07-14

### Added

* Added comprehensive malformed-instruction validation.
* Added undefined-encoding validation.
* Added validation of reserved and unused instruction fields.
* Added detection of missing extension words for multiword instructions.
* Added support for building the reusable `libt32` runtime library.

### Changed

* Updated the runtime to use the canonical 37-instruction T32 ISA.
* Centralized version information in `include/version.h`.
* Standardized the runtime build process.
* Established the external `t32` repository as the canonical ISA validation suite.

### Fixed

* Corrected the runtime build configuration.
* Removed residual assembler-specific build artifacts.
* Corrected project documentation and build targets.
* Standardized runtime executable generation.

### Runtime Components

The runtime implementation consists of:

```text
src/
    cli.c
    log.c
    main.c
    t32.c

include/
    t32.h
    t32_cli.h
    t32_log.h
    t32_opcodes.h
    version.h
```

### Build Outputs

The standard build produces:

```text
bin/
    t32-run.exe

lib/
    libt32.a
```

On non-Windows platforms, the runtime executable is produced as:

```text
bin/
    t32-run
```

### Validation Status

Current validation includes:

* All 37 implemented T32 instructions
* Reserved-opcode validation
* Malformed-instruction validation
* Undefined-encoding validation
* Instruction-decoding verification
* Runtime-execution validation

The external `t32` repository contains the complete ISA validation suite and remains the canonical source for instruction verification.


## t32-runx 0.0.1 - 2026-08-09

- Adds the first Windows-only `t32-runx` developer console.
- Reuses `libt32vm`; no second CPU implementation is introduced.
- Renders the canonical 80x25 T32 text display in a native Win32 window.
- Runs one vCPU in bounded instruction slices behind the Windows message pump.
- Automatically attaches `./disk.img` as disk0 when present.
- Keeps keyboard, mouse, timer, RTC, IRQ, networking, and SMP out of 0.0.1.
- Adds a source/build contract test for the intentionally narrow shim.
