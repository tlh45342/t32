#!/usr/bin/env python3
"""Host-side validation for the T32 string-to-int algorithm."""

from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

PROGRAM = "string-to-int.bin"
LOGFILE = "string-to-int.log"
LOAD_ADDRESS = 0x00001000
STACK_TOP = 0x0000F000
EXPECTED_CASES = 6

def initialize():
    for stream in (sys.stdin, sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8")
        except Exception:
            pass

def fail(message: str, output: str) -> int:
    print(f"❌ {message}")
    print("\n--- t32-run output ---")
    print(output.rstrip())
    print("--- end output ---")
    return 1


def check(condition: bool, success: str, failure: str) -> None:
    if not condition:
        raise AssertionError(failure)
    print(f"✅ {success}")


def parse_registers(output: str) -> dict[int, int]:
    registers: dict[int, int] = {}
    for number, value in re.findall(r"r(\d+)\s*=0x([0-9a-fA-F]{8})", output):
        registers[int(number)] = int(value, 16)
    return registers


def main() -> int:
    initialize()
    print("Running string-to-int validation...\n")

    if not Path(PROGRAM).is_file():
        print(f"❌ missing {PROGRAM}; run make first")
        return 1

    script = "\n".join([
        f"logfile {LOGFILE}",
        f"load {PROGRAM} 0x{LOAD_ADDRESS:08X}",
        f"set pc 0x{LOAD_ADDRESS:08X}",
        "run",
        "regs",
        "status",
        "logfile off",
        "quit",
        "",
    ])

    try:
        completed = subprocess.run(
            ["t32-run"],
            input=script,
            text=True,
            capture_output=True,
            timeout=20,
            check=False,
        )
    except FileNotFoundError:
        print("❌ t32-run was not found in PATH")
        return 1
    except subprocess.TimeoutExpired:
        print("❌ t32-run timed out")
        return 1

    output = completed.stdout + completed.stderr

    try:
        check(completed.returncode == 0, "t32-run exited successfully", f"t32-run exited with {completed.returncode}")
        check("loaded string-to-int.bin" in output, "binary loaded", "binary was not loaded")
        check("state=halted" in output, "machine halted", "machine did not halt")
        check("reason=HALT instruction" in output, "HALT instruction reached", "HALT was not the halt reason")

        registers = parse_registers(output)
        for register in range(7, 16):
            if register not in registers:
                raise AssertionError(f"r{register} missing from register dump")

        descriptions = {
            8: "empty string converts to zero",
            9: "zero converts to zero",
            10: "single digit converts correctly",
            11: "two-digit value converts correctly",
            12: "multi-digit value converts correctly",
            13: "leading zeroes are accepted",
        }
        for register, description in descriptions.items():
            check(
                registers[register] == 1,
                description,
                f"expected r{register}=1, got 0x{registers[register]:08X}",
            )

        check(
            registers[14] == EXPECTED_CASES,
            "all six test cases completed",
            f"expected r14={EXPECTED_CASES}, got {registers[14]}",
        )
        check(
            registers[15] == STACK_TOP,
            "stack pointer restored",
            f"expected r15=0x{STACK_TOP:08X}, got 0x{registers[15]:08X}",
        )
        check(
            registers[7] == 1,
            "algorithm result is PASS",
            f"expected r7=1, got 0x{registers[7]:08X}",
        )

    except AssertionError as error:
        return fail(str(error), output)

    print("\nPASS: string-to-int")
    return 0


if __name__ == "__main__":
    sys.exit(main())
