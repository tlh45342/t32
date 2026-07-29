# T32 Roadmap

## Current Focus

The current effort is establishing a validated algorithm library and an
automated validation framework.

### Immediate Objectives

- [ ] Complete Algorithm Tests
  - [ ] 00-memory-fill
  - [x] 01-memory-copy
  - [ ] 02-string-length
  - [ ] 03-string-compare
  - [ ] 04-int-to-string
  - [ ] 05-string-to-int

- [ ] Expand automated Python validation (`run_tests.py`)
- [ ] Define the initial T32 ABI

---

## Milestone 1 — Core ISA

- [x] System instructions
- [x] Data movement
- [x] Memory operations
- [x] Arithmetic
- [x] Logic
- [x] Compare
- [x] Branches
- [x] Stack
- [x] CALL / RET
- [ ] Undefined/reserved opcode review

## Milestone 2 — Validation Framework

- [x] Core instruction tests
- [x] Platform console tests
- [x] Algorithm test framework
- [x] Host-side validation
- [ ] Architecture tests
- [ ] System tests

## Milestone 3 — Runtime Algorithms

- [ ] memory-fill
- [x] memory-copy
- [ ] string-length
- [ ] string-compare
- [ ] int-to-string
- [ ] string-to-int

## Milestone 4 — Architecture

- [ ] ABI
- [ ] Calling convention
- [ ] Reset contract
- [ ] Vector table
- [ ] SVC
- [ ] Interrupt model

## Milestone 5 — Toolchain

- [x] t32-as
- [x] t32-run
- [ ] Relocatable object format
- [ ] t32-nm
- [ ] t32-ld
- [x] Initial t32-cc
- [ ] Runtime library

## Milestone 6 — Platform

- [x] Console
- [ ] Timer
- [ ] RTC
- [ ] Disk
- [ ] Keyboard
- [ ] Network

## Milestone 7 — System Software

- [ ] putc()
- [ ] puts()
- [ ] print_int()
- [ ] printf()
- [ ] Monitor
- [ ] Loader
