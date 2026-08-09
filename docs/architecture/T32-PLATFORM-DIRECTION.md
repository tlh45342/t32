# T32 Platform Direction — Interactive Runtime and Storage

Status: planning record  
Date: 2026-08-08

This document records the platform decisions made after the first successful
guest-visible virtual disk implementation in `t32-run` 0.0.6.

## 1. Immediate objective

The next visible machine milestone is a small interactive Windows host named
`t32-runx`.

`t32-runx` is intentionally a **semi-shim**. It exists to make T32 tangible and
to validate guest-visible device contracts before those contracts are consumed
by `t32-node`, VCONSOLE, VDISK, SWITCHYARD, and Foundry.

Initial target:

```text
Windows GUI executable
one T32 VM
one vCPU
existing 80x25 text display
./disk.img automatically attached as disk0 when present
no mouse
keyboard may follow the first display proof immediately
```

Cross-platform GUI work is explicitly not a goal of this executable.

## 2. Layering rule

Windows-specific code belongs in `t32-runx`, not in the T32 CPU or portable VM
core. A guest must not need to know whether its display, keyboard, disk or future
NIC is ultimately hosted by `t32-runx` or by Foundry infrastructure.

## 3. CPU scope

`t32-run` and `t32-runx` are single-vCPU machines.

A future VM architecture may contain multiple CPU instances sharing memory and
devices, but SMP introduces interrupt routing, synchronization, atomics, memory
ordering and CPU-startup questions. Those are deliberately deferred.

## 4. Interactive console

The first graphical host is still a text machine. The existing T32 80x25 display
MMIO remains authoritative. `t32-runx` merely renders that state into a Windows
window.

Mouse support is out of scope. Keyboard input should be introduced through a
small guest-visible keyboard device, initially suitable for ASCII/text polling.

## 5. Device service

The 0.0.6 disk implementation is synchronous. The expected evolution is a
general VM service path rather than a disk-specific idle hook:

```text
run guest instruction slice
service keyboard / disk / timer / RTC / future NIC
deliver pending IRQ/state changes
pump host events / refresh presentation
repeat
```

CPU HALT/idle and device service are separate concepts. Eventually a halted CPU
must be able to wait while a timer, disk or network device completes.

## 6. Local disk conventions

For the developer runner, convention is preferred over configuration initially:

```text
./disk.img   -> disk0
./disk2.img  -> disk1 (planned convenience)
```

The first `t32-runx` need only require `disk.img`. These filenames are host
conveniences, not guest-visible disk formats.

## 7. Storage layers

Do not conflate the following:

```text
host image file
    |
T32 block device        512-byte sectors / LBA
    |
disk layout             partition table / boot metadata
    |
filesystem              directories / files / allocation
    |
boot convention         firmware policy and boot-file location
```

`t32-run` 0.0.6 implements the block-device layer only.

A future `t32-disk` program should be the canonical command-line reference tool
for image creation, inspection, partitioning and filesystem preparation. Guppy
may implement a graphical view/editor against the same documented formats.

## 8. Filesystem direction

A tiny T32-native layout/filesystem may be useful for bootstrap and teaching.

EXT2 is a high-value early target because it is approachable, well understood,
Unix-like, and supported by existing host tooling. Firmware support, if added,
should initially be read-only and limited to what boot requires: locate, open
and read.

FAT12/FAT32 remain useful formats to understand. NTFS is possible later OS work,
but neither NTFS nor broad FAT support is a requirement for the initial BIOS.

## 9. Boot sources and persistent firmware settings

The first BIOS may simply boot disk0. The architecture should preserve a
boot-source abstraction so later firmware can consider disk0, disk1, and network.

Network boot is therefore a planned capability even though no protocol is yet
selected.

A future small NVRAM/firmware-settings facility may store preferences such as
boot order. It should not duplicate authoritative VM topology. RAM size, vCPU
count and attached devices are properties of the VM constructed by
`t32-runx`/`t32-node`/Foundry. There is no requirement to emulate legacy PC CMOS.

## 10. Near-term platform devices

After the first `t32-runx` display/disk proof, the expected high-value sequence
is:

1. keyboard
2. general device-service loop
3. timer
4. RTC
5. guest-requested shutdown/reset
6. IRQ completion behavior
7. asynchronous disk
8. virtual NIC
9. SWITCHYARD integration
10. network boot experiments

The order may change as BIOS, libc, compiler and kernel work expose stronger
dependencies.

## 11. Enterprise direction

`t32-runx` is intentionally not an enterprise runtime.

```text
T32 guest
    |
libt32vm + stable virtual-device contracts
    |
t32-node
    |
VCONSOLE / VDISK / SWITCHYARD
    |
Foundry
```

The local Windows runner is successful if it makes the machine easy to boot,
watch and interact with while proving contracts that survive that transition.
