# t32 — Instruction Set Test Suite

T32 is a small, custom instruction set used as a lightweight stand-in
for real architectures (ARM, x86) while validating the surrounding
orchestration and tooling layer. This repo holds the canonical
unit/smoke tests for T32 — used by the T32 toolchain directly, and
by Foundry-based integration testing.

See `example.md` for a walkthrough, and `Docs/` for architecture notes.

## T32 toolchain
- [t32-asm](https://github.com/tlh45342/t32-asm) — assembler (0.0.2 - 2026/7/12)
- [t32-cc](https://github.com/tlh45342/t32-cc) — C compiler [shim]
- [t32-ld](https://github.com/tlh45342/t32-ld) — linker [shim]
- [t32-node](https://github.com/tlh45342/t32-node) — emulator/execution node
- [t32-run](https://github.com/tlh45342/t32-run) — runner (0.0.3 - 2026/7/12)

## Orchestration ecosystem (Foundry)
- [foundry](https://github.com/tlh45342/foundry) — VM orchestrator
- [vmctl](https://github.com/tlh45342/vmctl) — CLI client
- [guppy](https://github.com/tlh45342/guppy) — disk/image tooling
- [switchyard](https://github.com/tlh45342/switchyard) — virtual network fabric
- [vconsole](https://github.com/tlh45342/vconsole) — console access

## Why T32 exists
T32's simplicity lets the orchestration layer above (Foundry, vmctl,
switchyard, guppy) be built and hardened quickly, before pointing the
same tooling at real architectures:
- [x64-vm](https://github.com/tlh45342/x64-vm) — x86 (real-mode) implementation
- [libvm](https://github.com/tlh45342/libvm) — ARMv7-A implementation

## Status
Early stage — test suite and toolchain are both under active development.

-TLH