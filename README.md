# T32

T32 is a small 32-bit virtual computer, toolchain, firmware stack, runtime, and
reference VM developed as part of the Foundry project.

## Current checkpoint — August 11, 2026

The working boot path is now:

```text
T32 machine
    |
embedded or external BIOS
    |
BOOT.BIN
    |
NEXT.BIN / Stage3 C monitor
    |
interactive guest environment
```

Verified platform capabilities include the T32 toolchain and `libt32`, reusable
single-vCPU `libt32vm`, `t32-run`, Windows `t32-runx`, 80x25 text display,
keyboard input, virtual block storage and T32D media, Bootinfo v0.2, BIOS
disk-read service v0.1, the compiler-built Stage3 monitor, and a guest-visible
RTC providing UTC epoch seconds.

The current Stage3 monitor (`0.0.13`) provides:

```text
help
version
bootinfo
mem
time
halt
```

`t32-runx` now embeds a default BIOS. Normal graphical startup is therefore
essentially: attach a disk image and start the machine. An external BIOS remains
available as an override for firmware development and testing.

## Near-term development direction

The next major milestone is external application execution:

```text
Stage3 `run`
    -> external HELLO.BIN
    -> external SORT.BIN
    -> return to Stage3
    -> application-facing SVC ABI
    -> monotonic timer / benchmark timing
    -> small executable-image contract
    -> additional standalone programs
```

An external directory such as `G:\X2` is deliberately useful: it proves that
installed T32 tools and libraries can build software without repository-local
dependencies. Cleaned examples can later move into `samples/`.

## Service layering direction

Application software should not accumulate dependencies on raw MMIO addresses or
BIOS implementation details.

```text
application
    |
libt32
    |
SVC ABI
    |
Stage3 / future supervisor
    |
BIOS services and MMIO
```

The existing BIOS service table remains a bootstrap/firmware interface, not the
future application SVC ABI. RTC remains the wall-clock source; benchmarking will
use a separate monotonic timer/counter.

T32D remains the bootstrap media format. A real filesystem remains planned, with
ext2 a target after the external-program/SVC/timing milestones.

`t32-run` and `t32-runx` intentionally remain single-vCPU developer/reference
hosts. Multi-VM lifecycle belongs in the later `t32-node` architecture and
Foundry integration.

## Documentation

Start with `docs/README.md`. Current implementation status is in `STATUS.md`;
architecture, ABI, runtime, development, and project documents live under
`docs/`.

Canonical documentation belongs in these maintained files. Temporary
patch-specific README files should not accumulate at the repository root.
