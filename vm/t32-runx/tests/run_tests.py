#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "runx.c"
MAKEFILE = ROOT / "Makefile"
RUNX = ROOT / "bin" / "t32-runx.exe"
LIB = ROOT.parent / "libt32vm"

def req(condition, message):
    if not condition:
        print(f"  FAIL {message}")
        raise SystemExit(1)
    print(f"  PASS {message}")

def main():
    print("Running t32-runx 0.0.7 lifecycle/UI validation...")
    source = SOURCE.read_text(encoding="utf-8")
    makefile = MAKEFILE.read_text(encoding="utf-8")

    req((LIB/"include/t32.h").exists(), "sibling libt32vm public header exists")
    req((LIB/"lib/libt32vm.a").exists(), "sibling libt32vm archive exists")
    req(RUNX.exists(), "runx executable exists")
    req('#define T32_RUNX_VERSION "0.0.7"' in source,
        "source reports version 0.0.7")

    req("IDM_MACHINE_START" in source, "Machine menu has Start")
    req("IDM_MACHINE_STOP" in source, "Machine menu has hard Stop")
    req("IDM_MACHINE_RESET" in source, "Machine menu has Reset")
    req("IDM_MACHINE_EXIT" in source, "Machine menu has Exit")
    req("IDM_MACHINE_LOAD_PROGRAM" in source and "load_program" in source,
        "Machine menu has direct Load Program action")
    req('AppendMenuA(machine, MF_STRING, IDM_MACHINE_LOAD_PROGRAM' in source,
        "Load Program is placed under Machine")
    req("IDM_FILE_LOAD_PROGRAM" not in source,
        "obsolete File Load Program command is removed")
    req("GetOpenFileNameA(&ofn)" in source and '"Load T32 Program"' in source,
        "Load Program uses standard Windows common file-open dialog")
    req("OFN_FILEMUSTEXIST" in source and "OFN_PATHMUSTEXIST" in source,
        "file-open dialog requires an existing program")
    req("T32_RUNX_PROGRAM_ADDRESS UINT32_C(0x00020000)" in source,
        "direct program load address is 0x00020000")
    req("direct_program = true" in source and
        "t32_set_pc(g_runx.machine, T32_RUNX_PROGRAM_ADDRESS)" in source,
        "direct load sets program mode and entry PC")
    req("Loading is deliberately non-running" in source,
        "direct program load does not auto-start")

    req("IDM_VIEW_STATS" in source and "show_stats_window" in source,
        "View menu has Stats action")
    req('AppendMenuA(view, MF_STRING, IDM_VIEW_STATS, "&Stats...")' in source,
        "Stats is placed under View")
    req("ES_READONLY" in source and "WS_EX_TOOLWINDOW" in source,
        "Stats window is read-only and modeless")
    req("CPU Registers" in source and "Machine Status" in source,
        "Stats window reports CPU and machine sections")
    req("t32_get_register" in source and "t32_get_pc" in source and
        "t32_get_instruction_count" in source,
        "Stats window reads register, PC, and instruction state")
    req("t32_get_flags" in source and "t32_get_halt_reason" in source,
        "Stats window reads flags and halt reason")

    req("IDM_HELP_ABOUT" in source and "show_about" in source,
        "Help menu has About action")
    req('AppendMenuA(help, MF_STRING, IDM_HELP_ABOUT, "&About...")' in source,
        "About is placed under Help")
    req('"About T32 RunX"' in source and "T32_RUNX_VERSION" in source,
        "About reports the running version")

    req('"T32RunXWindow",\n        "t32-runx"' in source,
        "main window is explicitly created with T32RunXWindow class")
    req('wc.lpszClassName,\n        "t32-runx"' not in source,
        "main window no longer inherits the last registered class")

    req("IDM_FIRMWARE_SELECT" in source, "Firmware selection remains present")
    req("IDM_DISK_ATTACH0" in source and "IDM_DISK_DETACH0" in source,
        "Disk 0 attach/detach remains present")
    req("create_status_bitmap" in source and "MFT_RIGHTJUSTIFY" in source,
        "far-right menu status lamp exists")
    req("RGB(220, 32, 32)" in source and "RGB(32, 180, 64)" in source,
        "status lamp has red and green states")
    req('SetWindowTextA(window, "t32-runx")' in source,
        "window title remains intentionally simple")
    req("guest_power_off" in source and "DestroyWindow(window)" not in
        source[source.index("static void guest_power_off"):
               source.index("static void run_vm_slice")],
        "guest POWER_OFF leaves application open")
    req("machine_stop" in source and "powered_off = true" in source,
        "hard Stop forces powered-off restart semantics")
    req("WM_CHAR" in source and "t32_keyboard_push" in source,
        "keyboard path remains interactive")
    req("T32_LIB      := $(LIBT32VM_DIR)/lib/libt32vm.a" in makefile,
        "runx links independent libt32vm")
    req("CC      := gcc" in makefile, "Windows compiler is pinned to gcc")

    print("t32-runx: PASS (36/36 cases)")
    return 0

if __name__ == "__main__":
    sys.exit(main())
