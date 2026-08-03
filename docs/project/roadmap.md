# T32 Roadmap

## Current Focus

The current focus is the incremental construction of `t32-cc` on top of the validated T32 ISA, ABI, object format, linker, runtime library, startup object, and VM.

The compiler is being expanded one language concept at a time. Each milestone must pass positive and negative tests and complete the end-to-end path from C source to executed T32 binary.

---

## Completed Foundation

### Architecture and Execution

- [x] 32-bit T32 programmer model
- [x] Core instruction implementation through CALL/RET
- [x] Register-tested conditional branches
- [x] Stack and nested-call validation
- [x] Virtual machine and command monitor
- [x] Host-side VM library renamed to `libt32vm.a`
- [x] Dedicated `t32-run` smoke-test suite

### Toolchain

- [x] `t32-as`
- [x] T32OBJ relocatable object format
- [x] `t32-nm`
- [x] `t32-ld`
- [x] T32AR static archive format
- [x] `t32-ar`
- [x] Root monorepo build and install orchestration
- [x] Local-checkout tool preference during builds and tests

### ABI and Runtime

- [x] ABI 0.1 documentation
- [x] Argument and return-register conventions
- [x] Caller-saved and callee-saved conventions
- [x] Stack restoration and alignment rules
- [x] Nested-call and stack-argument rules
- [x] Eight-part ABI validation suite
- [x] `libt32.a` static target runtime archive
- [x] Sixteen runtime routines
- [x] Archive-member selection and omission validation
- [x] ABI-compliant `crt0.o`
- [x] `_start -> main -> HALT` runtime path
- [x] User-local runtime installation under `lib/t32`

### Compiler

- [x] Compiler driver modes: `-S`, `-c`, and full link
- [x] Default output naming
- [x] Quiet success and opt-in `-v`
- [x] ABI-compliant `main`
- [x] Constant integer return
- [x] Relocatable object generation
- [x] Automatic linking with `crt0.o` and `libt32.a`
- [x] End-to-end compiler execution validation
- [x] One initialized stack-based local integer
- [x] Initial compiler symbol lookup
- [x] Negative tests for syntax, missing inputs, missing runtime, and partial outputs

---

## Immediate Compiler Milestones

### Assignment and Mutation

- [ ] Parse `identifier = integer-literal;`
- [ ] Look up the destination symbol
- [ ] Store the replacement value in the existing stack slot
- [ ] Reject assignment to undeclared identifiers
- [ ] Reject unsupported right-hand-side expressions
- [ ] Preserve stack restoration and ABI behavior

### Integer Expressions

- [ ] Addition
- [ ] Subtraction
- [ ] Multiplication
- [ ] Division
- [ ] Parenthesized expressions
- [ ] Operator precedence
- [ ] Literal and local-variable operands
- [ ] Temporary-register strategy

### Comparisons

- [ ] Equality and inequality
- [ ] Signed less-than and greater-than
- [ ] Signed less-than-or-equal and greater-than-or-equal
- [ ] Decide compiler lowering using current explicit register control flow
- [ ] Measure whether existing `CMP/CMPI` assist generated code

### Selection and Iteration

- [ ] `if`
- [ ] `if/else`
- [ ] `while`
- [ ] `do/while`
- [ ] `for`
- [ ] nested control flow
- [ ] label generation

### Functions and Data

- [ ] Multiple functions
- [ ] Function calls and parameters
- [ ] Recursion
- [ ] Multiple locals
- [ ] Nested scopes
- [ ] Pointers
- [ ] Arrays
- [ ] Structures
- [ ] Larger and aggregate return values
- [ ] Variadic-function convention

---

## Architecture Refinement

These questions are documented but intentionally do not block the compiler:

- [ ] Complete instruction flag-effects table
- [ ] Formalize whether C/Z/N/V become a packed `STATUS` value
- [ ] Evaluate `MRS` and `MSR`
- [ ] Define complete interrupt and `IRET` context
- [ ] Document exact shift-count behavior
- [ ] Document exact alignment behavior
- [ ] Document multiply overflow and discarded high-word semantics
- [ ] Document signed division truncation and divide faults
- [ ] Evaluate `MULH/MULHU` from workload evidence
- [ ] Evaluate `REM/REMU` from workload evidence
- [ ] Create instruction-frequency and recurring-sequence analysis tools

See:

```text
docs/architecture/open-questions.md
```

---

## Runtime and Standard Library

- [x] Core memory routines
- [x] Core string routines
- [x] Initial conversion routines
- [ ] Runtime headers and public include installation
- [ ] Character output
- [ ] String output
- [ ] Integer formatting
- [ ] `printf` subset
- [ ] Dynamic memory support
- [ ] Expanded compiler/runtime integration tests

---

## Platform

- [x] Initial memory-mapped text-console validation
- [ ] BIOS or firmware startup
- [ ] Console/KVM integration
- [ ] Timer
- [ ] RTC
- [ ] Block storage
- [ ] Keyboard
- [ ] Network
- [ ] Device ABI documentation

---

## System Software

- [ ] Loader
- [ ] System monitor
- [ ] Interrupt dispatcher
- [ ] Task/context switching
- [ ] T32FS
- [ ] Operating environment
- [ ] Small C programs exercising the complete platform

---

## Long-Term Measurement Goals

T32 should eventually report:

- instruction-frequency histograms;
- common instruction pairs;
- repeated three-to-six-instruction sequences;
- code size per C construct;
- estimated cycle cost per C construct;
- instruction usage by compiler, runtime, firmware, system software, and applications.

ISA additions or removals should be guided by this evidence rather than fashion or intuition.
