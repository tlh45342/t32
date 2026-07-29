# T32 Documentation

This directory contains the architectural, testing, and project-status documentation for the T32 virtual processor.

## Document roles

| Document | Purpose |
|---|---|
| [programmer-model.md](programmer-model.md) | CPU-visible registers, flags, state, and execution model |
| [instruction-set.md](instruction-set.md) | Instruction inventory, syntax, and currently verified behavior |
| [instruction-encoding.md](instruction-encoding.md) | Current encoding evidence and rules that still require formalization |
| [memory-map.md](memory-map.md) | Test-environment addresses and future architectural memory regions |
| [testing-guide.md](testing-guide.md) | How the regression and conformance tests are organized and run |
| [implementation-status.md](implementation-status.md) | Current assembler/runtime/test coverage by instruction |
| [design-decisions.md](design-decisions.md) | Decisions that define intended T32 behavior |
| [roadmap.md](roadmap.md) | Ordered development plan |
| [related-projects.md](related-projects.md) | Repositories in the wider Foundry/T32 ecosystem |

## Authority and evidence

The architectural documents describe intended behavior. The implementation and tests must eventually agree with them.

At the current stage, some behaviors are proven only by tests and some architectural details are still provisional. Documents use these labels:

- **Verified** — demonstrated by the supplied assembler/runtime tests.
- **Specified** — intentionally defined as part of the architecture.
- **Provisional** — current convention that may change.
- **Open** — requires an explicit design decision or additional validation.

When documentation and implementation differ, record the discrepancy in `implementation-status.md` and resolve it deliberately. Do not silently redefine the architecture from a single test result.

## Current snapshot

The supplied test tree contains 36 instruction-test directories. All have smoke tests. Expanded multi-case validation currently exists for:

- ADD
- ADDI
- SUB
- SUBI
- CMP
- CMPI

The next planned validation phase covers logical, shift, multiplication, and division instructions.
