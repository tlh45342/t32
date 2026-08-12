# t32-runx 0.0.8

`t32-runx` is the Windows-only, single-vCPU interactive developer host for T32.
It reuses the canonical sibling `../libt32vm`; there is no duplicate CPU/MMIO
implementation.

## Menus

```text
Machine
  Start
  Stop
  Reset
  ----------------
  Load Program...
  ----------------
  Exit

Firmware
  Select BIOS...

Disk
  Attach Disk 0...
  Detach Disk 0

View
  Stats...

Help
  About...

                                               ●
```

The far-right lamp is green while the VM executes and red while it is stopped
or powered off.

## Direct program development

`Machine -> Load Program...` uses the standard Windows common file-open dialog.
A selected flat `.bin` program is loaded at `0x00020000`, PC is set to the same
address, and the VM remains stopped until `Machine -> Start`.

This is the GUI equivalent of:

```text
load hello.bin 0x00020000
set pc 0x00020000
run
```

Selecting BIOS firmware switches back to the normal firmware + Disk 0 boot
workflow.

## CPU Stats

`View -> Stats...` opens a modeless read-only window containing r0-r15, PC,
state, instruction count, flags, and halt/fault reason. It refreshes during
execution and after lifecycle changes.

## About

`Help -> About...` displays the running `t32-runx` version so development builds
can be identified immediately.

## 0.0.7 fix

0.0.6 registered the main window class and the Stats window class, but then
created the visible main window through the last value left in
`WNDCLASS.lpszClassName`. At that point the last class was the Stats class, so
the visible application window received the Stats window procedure instead of
the normal application window procedure. Menu commands therefore never reached
the real `WM_COMMAND` dispatcher.

0.0.7 explicitly creates the main window with `T32RunXWindow`. This repairs the
Stats command and the standard file-open command path.

## Current scope

- Windows only
- one vCPU
- 80x25 text display
- polling ASCII keyboard
- synchronous disk
- direct flat-binary development loading
- read-only CPU Stats window
- guest POWER_OFF / RESET platform control
- no mouse
- no timer/RTC/IRQ yet
- no networking yet


## 0.0.8 default BIOS policy

`t32-runx` now has two firmware modes:

```text
A  embedded BIOS (default)
B  external BIOS file override
```

Mode A requires no firmware file at runtime. During the build,
`../../firmware/bios/bios.bin` is converted into a generated C byte array and
linked into `t32-runx.exe`.

The Firmware menu provides both:

```text
Use Embedded BIOS (A)
Select External BIOS... (B)
```

Supplying a firmware filename as the first command-line argument continues to
select the external/file mode for compatibility.

The embedded BIOS is a convenience/default, not a replacement for firmware
development. Rebuilding `t32-runx` after `bios.bin` changes refreshes the
embedded image automatically.
