# T32 Memory Map

## Current firmware/boot map

| Address | Use | Status |
|---:|---|---|
| `0x00001000` | BIOS / early firmware | Current boot convention |
| `0x00002000` | 72-byte Bootinfo v0.2 record | Boot ABI v0.2 |
| `0x0000F000` | temporary BIOS/BOOT stack top | Provisional firmware convention |
| `0x00010000` | BOOT.BIN load address and entry point | Boot ABI v0.1 |
| `0x90000000` | 80x25 text framebuffer | Current platform MMIO |
| `0x90001000` | disk0 controller | Current platform MMIO |
| `0x90002000` | keyboard | Current platform MMIO |
| `0x90003000` | RTC | Current platform MMIO |
| `0x90004000` | platform control / discovery | Current platform MMIO |

The Bootinfo structure and `r0` handoff are specified in
`docs/abi/T32-BOOT-ABI.md`. Device register layouts remain documented in
`docs/runtime/mmio.md` and the canonical `libt32vm` header while the platform
specification is still evolving.

## Required architectural regions still to settle

A later versioned machine map should additionally define:

```text
reset and exception vectors
boot ROM mapping
post-boot vector table
general RAM ownership / usable ranges
system monotonic timer/counter
interrupt controller
network controller
reserved regions
```

## Open decisions

- final reset-vector and ROM mapping;
- alignment requirements beyond current instruction/data behavior;
- unmapped-access behavior across all devices;
- read-only and executable regions;
- MMIO ordering and access-width rules;
- ROM reclamation or remapping after boot;
- post-boot ownership of the temporary firmware stack and Bootinfo page.
