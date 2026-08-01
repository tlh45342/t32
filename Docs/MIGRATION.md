# Documentation Migration

Replace the existing `docs/` contents with this kit rather than merging file by file.

## Retained and refactored

- programmer model
- instruction set
- instruction encoding
- conditional branches
- memory map
- design decisions
- testing guide
- implementation status
- roadmap
- related projects

## Consolidated or retired

- the old `testing.md` outline is incorporated into `development/testing-guide.md`;
- the plain URL list is incorporated into `project/related-projects.md`;
- `tre important.md` is preserved as `archive/foundry-control-plane-note.md`;
- the old root README is replaced by the new documentation index.

## Added

- `runtime/abi.md`
- `runtime/algorithms.md`
- `runtime/bios.md`
- `runtime/console.md`
- `runtime/mmio.md`
- `runtime/svc.md`
- `project/ecosystem.md`

## Important provisional material

The ABI register-preservation rules, final stack layout, BIOS reset contract, MMIO addresses, supervisor-call numbers, interrupt frame, and final architectural memory map are not yet frozen.
