# T32 Memory Map

## Current validation map

| Address | Use | Status |
|---:|---|---|
| `0x00001000` | Program load address and entry point | Provisional test convention |
| `0x00002000` | Scratch data | Provisional test convention |
| `0x00003000` | Initial stack pointer | Provisional test convention |

These addresses describe the present test environment, not the final machine map.

## Required architectural regions

A versioned machine map should eventually define:

```text
reset and exception vectors
boot ROM
loaded program region
RAM
stack convention
memory-mapped console
interrupt controller
system timer
RTC
storage controller
network controller
reserved regions
```

## Open decisions

- address-space size and legal RAM ranges;
- byte order;
- alignment requirements;
- unmapped-access behavior;
- read-only and executable regions;
- MMIO ordering and access width;
- ROM reclamation or remapping after boot.

Device register assignments belong in `runtime/mmio.md` until incorporated into a versioned platform specification.
