# T32 Documentation

This directory is the maintained documentation set for the T32 virtual processor and its early software environment.

## Documentation layers

- `architecture/` defines the processor-visible architecture.
- `runtime/` defines conventions used by software built for T32.
- `development/` explains implementation status, testing, decisions, and planned work.
- `project/` places T32 within the wider Foundry ecosystem.
- `archive/` preserves useful non-normative notes that do not belong in the active specification.

## Authority

Documents use these terms:

- **Specified** — intentionally part of the architecture or software contract.
- **Verified** — demonstrated by current tests.
- **Provisional** — current convention that may still change.
- **Open** — requires a deliberate decision or additional validation.

The architecture documents describe intended behavior. Tests and implementations must converge on those documents. A single implementation result does not silently redefine the architecture.

## Recommended reading order

1. `architecture/programmer-model.md`
2. `architecture/instruction-set.md`
3. `architecture/instruction-encoding.md`
4. `architecture/conditional-branches.md`
5. `architecture/memory-map.md`
6. `runtime/abi.md`
7. `runtime/algorithms.md`
8. `development/testing-guide.md`
9. `development/implementation-status.md`
10. `development/roadmap.md`
