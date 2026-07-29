# T32 Design Notes

## Layered Development

The project intentionally develops upward through validated layers.

Instruction
    ↓
Algorithm
    ↓
Architecture
    ↓
Platform
    ↓
System

Every abstraction should be built upon previously validated components.

## Validation Philosophy

Each test has two independent validators:

1. Guest validation
   - Executes the algorithm.
   - Verifies its own results.
   - Reports PASS/FAIL.

2. Host validation
   - Parses VM output.
   - Confirms expected architectural state.
   - Returns a process exit code.

This separation reduces ambiguity and makes the entire suite suitable for
automation.

## Toolchain Philosophy

The assembler comes first.

The object format should be defined before the linker.

Planned progression:

t32-as
    ↓
Relocatable object format
    ↓
t32-nm
    ↓
t32-ld
    ↓
Runtime library
