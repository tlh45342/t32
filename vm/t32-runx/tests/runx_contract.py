#!/usr/bin/env python3
from pathlib import Path
import os
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "runx.c"
MAKEFILE = ROOT / "Makefile"

def req(condition, message):
    if not condition:
        print(f"  FAIL {message}")
        raise SystemExit(1)
    print(f"  PASS {message}")

def main():
    print("Running t32-runx 0.0.1 contract validation...")
    source = SOURCE.read_text(encoding="utf-8")
    makefile = MAKEFILE.read_text(encoding="utf-8")

    req('#define T32_RUNX_VERSION "0.0.1"' in source,
        "runx source declares version 0.0.1")
    req("T32_VIDEO_COLUMNS" in source and "T32_VIDEO_ROWS" in source,
        "runx uses canonical T32 display geometry")
    req("t32_read_memory" in source and "T32_VIDEO_BASE" in source,
        "runx renders the existing display MMIO")
    req('file_exists("disk.img")' in source,
        "runx probes default disk.img")
    req('t32_disk_attach(g_runx.machine, "disk.img")' in source,
        "runx attaches disk.img as the current disk")
    req("T32_RUNX_SLICE_INSTRUCTIONS" in source and "t32_step(" in source,
        "runx executes bounded VM slices")
    req("SetTimer" in source and "WM_TIMER" in source,
        "runx keeps execution behind the Windows message pump")
    req("WM_PAINT" in source and "TextOutA" in source,
        "runx paints an 80x25 text window")
    req("WM_KEYDOWN" not in source and "WM_CHAR" not in source,
        "0.0.1 does not accidentally define keyboard semantics")
    req("WM_MOUSE" not in source,
        "0.0.1 contains no mouse device path")
    req("-mwindows" in makefile and "-lgdi32" in makefile,
        "Windows GUI link flags are present")
    req("ifeq ($(OS),Windows_NT)" in makefile,
        "runx build remains Windows-only")
    req("t32-runx" in makefile,
        "make/install know about t32-runx")

    print("t32-runx: PASS (13/13 contract cases)")
    return 0

if __name__ == "__main__":
    sys.exit(main())
