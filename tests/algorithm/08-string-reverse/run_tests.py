#!/usr/bin/env python3
"""Host-side validation for the T32 string-reverse algorithm."""

from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

PROGRAM = "string-reverse.bin"
LOGFILE = "string-reverse.log"
LOAD_ADDRESS = 0x00001000
GUARD_ADDRESS = 0x00001008
GUARD_LENGTH = 13
EXPECTED_GUARDED_REGION = bytes([0xA5]) + b"yrdnuoF\x00" + bytes([0xCC]) * 3 + bytes([0x5A])

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


def parse_memory(output: str, start: int, length: int) -> bytes:
    memory: dict[int, int] = {}
    for address_text, byte_text in re.findall(
        r"^0x([0-9a-fA-F]{8}):((?: [0-9a-fA-F]{2})+)",
        output,
        re.MULTILINE,
    ):
        address = int(address_text, 16)
        for offset, value in enumerate(byte_text.split()):
            memory[address + offset] = int(value, 16)

    missing = [address for address in range(start, start + length) if address not in memory]
    if missing:
        raise AssertionError(f"memory dump missing address 0x{missing[0]:08X}")
    return bytes(memory[address] for address in range(start, start + length))


def main() -> int:
    initialize()
    print("Running string-reverse validation...\n")

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
        f"e 0x{GUARD_ADDRESS:08X} {GUARD_LENGTH}",
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
        check("loaded string-reverse.bin" in output, "binary loaded", "binary was not loaded")
        check("state=halted" in output, "machine halted", "machine did not halt")
        check("reason=HALT instruction" in output, "HALT instruction reached", "HALT was not the halt reason")

        registers = parse_registers(output)
        for register in range(7, 16):
            if register not in registers:
                raise AssertionError(f"r{register} missing from register dump")

        descriptions = {
            8: "empty string reversed correctly",
            9: "one-character string reversed correctly",
            10: "even-length string reversed correctly",
            11: "odd-length string reversed correctly",
            12: "normal word reversed correctly",
            13: "phrase with spaces reversed correctly",
        }
        for register, description in descriptions.items():
            check(registers[register] == 1, description, f"expected r{register}=1, got 0x{registers[register]:08X}")

        check(registers[14] == 6, "all six test cases completed", f"expected r14=6, got {registers[14]}")
        check(registers[15] == 0x0000F000, "stack pointer restored", f"expected r15=0x0000F000, got 0x{registers[15]:08X}")
        check(registers[7] == 1, "algorithm result is PASS", f"expected r7=1, got {registers[7]}")

        region = parse_memory(output, GUARD_ADDRESS, GUARD_LENGTH)
        check(
            region == EXPECTED_GUARDED_REGION,
            "host observed reversed word and intact guards",
            f"unexpected guarded destination bytes: {region.hex(' ')}",
        )

    except AssertionError as error:
        return fail(str(error), output)

    print("\nPASS: string-reverse")
    return 0


if __name__ == "__main__":
    sys.exit(main())
