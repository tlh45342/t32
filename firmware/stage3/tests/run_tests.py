from __future__ import annotations

import re
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
WORK = ROOT / "tests" / "build"


def passed(message: str) -> None:
    print(f"  PASS {message}")


def require(condition: bool, message: str, output: str = "") -> None:
    if not condition:
        print(f"  FAIL {message}")
        if output:
            print(output)
        raise SystemExit(1)
    passed(message)


def bootinfo() -> bytes:
    words = (
        0x42323354, 72, 0, 2, 1,
        0, 0x00100000, 0, 512, 2048,
        0x90000000, 80, 25, 0x90001000, 0x90004000,
        0x00010000, 0, 0,
    )
    return struct.pack("<18I", *words)


def main() -> int:
    if len(sys.argv) != 4:
        print("usage: run_tests.py <t32-run> <next.bin> <next.map>")
        return 2

    runner = Path(sys.argv[1]).resolve()
    image = Path(sys.argv[2]).resolve()
    map_file = Path(sys.argv[3]).resolve()

    print("Running T32 Stage3 Monitor 0.0.13 validation...")
    require(runner.is_file(), "t32-run exists")
    require(image.is_file() and image.stat().st_size > 0, "next.bin exists and is non-empty")
    require(map_file.is_file() and map_file.stat().st_size > 0, "link map exists and is non-empty")
    map_text = map_file.read_text(encoding="utf-8", errors="replace")
    require("_start" in map_text and "0x00020000" in map_text,
            "C stage entry is linked at 0x00020000", map_text)
    require("main" in map_text, "compiler-generated main is linked into NEXT.BIN", map_text)

    WORK.mkdir(parents=True, exist_ok=True)
    info = WORK / "bootinfo.bin"
    info.write_bytes(bootinfo())

    commands = "\n".join([
        f"load {image} 0x20000",
        f"load {info} 0x2000",
        "set r0 0x2000",
        "set pc 0x20000",
        "key \\rversiox\\bn\\rhelp\\rmem\\rtime\\rbootinfo\\rhalt\\r",
        "run",
        "regs",
        "display",
        "quit",
        "",
    ])
    result = subprocess.run(
        [str(runner)], input=commands, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        cwd=ROOT, check=False, timeout=15,
    )
    require(result.returncode == 0, "standalone stage-3 execution succeeds", result.stdout)
    require("T32 Stage3 Monitor 0.0.13" in result.stdout, "C stage-3 banner appears", result.stdout)
    require("time     show RTC UTC epoch seconds" in result.stdout,
            "NEXT.BIN identifies interactive monitor payload", result.stdout)
    require("T32 Stage3 Monitor 0.0.13" in result.stdout, "interactive C monitor starts", result.stdout)
    require("help     show commands" in result.stdout, "help command executes", result.stdout)
    require("Stage3 load address 0x00020000" in result.stdout, "mem command executes", result.stdout)
    match = re.search(r"RTC epoch:\s*([0-9]+)", result.stdout)
    require(match is not None, "time command reports RTC epoch", result.stdout)
    if match is not None:
        epoch = int(match.group(1))
        require(abs(epoch - int(__import__("time").time())) <= 10,
                "time command tracks host UTC seconds", result.stdout)
    require("Unknown command" not in result.stdout, "blank line and edited command parse cleanly", result.stdout)
    require("Bootinfo v0.2 handoff OK" in result.stdout, "C stage accepts Bootinfo handoff", result.stdout)
    require("r0 =0x0000002a" in result.stdout.lower(), "C main return value remains in r0", result.stdout)

    bad = WORK / "bad-bootinfo.bin"
    bad.write_bytes(bytes(72))
    bad_commands = "\n".join([
        f"load {image} 0x20000",
        f"load {bad} 0x2000",
        "set r0 0x2000",
        "set pc 0x20000",
        "run",
        "display",
        "quit",
        "",
    ])
    bad_result = subprocess.run(
        [str(runner)], input=bad_commands, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        cwd=ROOT, check=False, timeout=15,
    )
    require(bad_result.returncode == 0, "invalid-handoff monitor execution succeeds", bad_result.stdout)
    require("C STAGE3 HANDOFF INVALID" in bad_result.stdout, "stage-3 rejects invalid Bootinfo", bad_result.stdout)

    print("t32-stage3: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
