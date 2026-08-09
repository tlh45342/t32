# libt32vm

`libt32vm` is the canonical host-native T32 virtual-machine core.

It is deliberately independent of any particular host application. The same
library is consumed by:

```text
t32-run     command-line monitor / headless reference host
t32-runx    Windows interactive developer host
t32-node    planned service/node host
```

The library currently owns:

- one T32 vCPU model
- guest RAM and register state
- instruction execution
- 80x25 text-display MMIO
- synchronous virtual-disk MMIO
- polling keyboard MMIO/FIFO
- platform-control MMIO for guest POWER_OFF / RESET requests

`t32-run` and `t32-runx` provide host policy and presentation; they do not own
separate CPU/device implementations.

## Build and test

```text
make
make test
```

## Install

```text
make install
```

Installs:

```text
lib/libt32vm.a
include/t32.h
include/t32_opcodes.h
```

## Current scope

`libt32vm` is currently single-vCPU. SMP/multi-vCPU, timer, RTC, IRQ delivery,
asynchronous disk service, and virtual networking are later platform work.
