# t32-runx 0.0.6

`t32-runx` is the intentionally Windows-only, single-vCPU interactive
developer host for T32.

It links the canonical sibling `../libt32vm/lib/libt32vm.a`. It does not contain
a second CPU or MMIO implementation.

## Application shell

```text
Machine
  Start
  Stop
  Reset
  Exit

Firmware
  Select BIOS...

Disk
  Attach Disk 0...
  Detach Disk 0

                                               ●
```

The far-right status lamp is:

```text
green   VM executing
red     VM stopped/powered off
```

The window title is deliberately just `t32-runx`; firmware paths, disk paths,
and state are no longer packed into the title bar.

## Lifecycle semantics

`Start`
: Starts the configured machine. After Stop, HALT/error, or guest POWER_OFF,
  Start performs a fresh boot from the selected firmware.

`Stop`
: Immediate host-enforced power cut. The guest is not notified and receives no
  cleanup opportunity. The last framebuffer remains visible for inspection.

`Reset`
: Recreates the machine from the selected firmware and disk and immediately
  starts it.

Guest `POWER_OFF`
: Powers off the virtual machine but leaves the `t32-runx` application open.
  This is intentionally different from **Exit**.

`Exit`
: Closes the Windows application.

## Firmware and disk

Firmware remains replaceable:

```text
Firmware -> Select BIOS...
```

If `disk.img` exists in the current working directory it remains the initial
Disk 0 convention. A different image can be selected interactively.

## Current scope

- Windows only
- one vCPU
- 80x25 text display
- polling ASCII keyboard
- synchronous disk
- guest POWER_OFF / RESET platform control
- no mouse
- no timer/RTC/IRQ yet
- no networking yet


## 0.0.6 cleanup

- Status lamp is enabled/display-only so Windows renders red/green instead of grey.
- Clicking the lamp intentionally does nothing.
- Disk detach no longer has an unused window parameter.
- `make install` no longer forces a relink merely because a phony library helper ran.


## Direct program development

`File -> Load Program...` loads a flat T32 binary at `0x00020000`, sets PC to
that same address, and leaves the machine stopped. Choose `Machine -> Start`
to execute it. This is the GUI equivalent of the `t32-run` monitor sequence:

```text
load hello.bin 0x00020000
set pc 0x00020000
run
```

Selecting BIOS firmware switches back to normal firmware boot mode. The direct
program path does not replace BIOS + Disk 0 boot; both workflows remain
available.

## CPU Stats window

`View -> Stats...` opens a modeless, read-only CPU state window showing:

- r0-r15;
- PC;
- machine state;
- executed instruction count;
- carry, zero, negative, and overflow flags;
- halt/fault reason.

The window refreshes while the VM is running and after lifecycle transitions.
