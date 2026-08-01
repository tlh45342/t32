from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
SCRIPT = HERE / "test.script"
LOG = HERE / "memory-compare.log"

EXPECTED_REGISTERS = {
    7: 1,
    8: 0,
    9: 1,
    10: 1,
    11: 1,
    12: 0,
    14: 5,
    15: 0x0000F000,
}

def initialize():
    for stream in (sys.stdin, sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8")
        except Exception:
            pass

def fail(message: str, output: str = "") -> None:
    print(f"❌ {message}")
    if output:
        print("\n--- t32-run output ---")
        print(output.rstrip())
        print("--- end output ---")
    raise SystemExit(1)

def passed(message: str) -> None:
    print(f"✅ {message}")

def register_value(text: str, register: int) -> int | None:
    patterns = (
        rf"\br{register}\s*=\s*(0x[0-9a-fA-F]+|\d+)\b",
        rf"\bR{register}\s*=\s*(0x[0-9a-fA-F]+|\d+)\b",
    )
    for pattern in patterns:
        match = re.search(pattern, text)
        if match:
            return int(match.group(1), 0)
    return None

def require_register(text: str, register: int, expected: int, label: str) -> None:
    actual = register_value(text, register)
    if actual is None:
        fail(f"r{register} was not found while checking {label}", text)
    if actual != expected:
        fail(
            f"{label}: expected r{register}=0x{expected:08X}, "
            f"got 0x{actual:08X}",
            text,
        )
    passed(label)

def main() -> int:
    initialize()
    print("Running memory-compare validation...\n")

    if LOG.exists():
        LOG.unlink()

    try:
        script_text = SCRIPT.read_text(encoding="utf-8")
    except OSError as exc:
        fail(f"cannot read {SCRIPT.name}: {exc}")

    try:
        completed = subprocess.run(
            ["t32-run"],
            input=script_text,
            text=True,
            capture_output=True,
            cwd=HERE,
            check=False,
        )
    except FileNotFoundError:
        fail("t32-run was not found in PATH")

    console = (completed.stdout or "") + (completed.stderr or "")
    log_text = LOG.read_text(encoding="utf-8", errors="replace") if LOG.exists() else ""
    output = console + "\n" + log_text

    if completed.returncode != 0:
        fail(f"t32-run exited with status {completed.returncode}", output)
    passed("t32-run exited successfully")

    if not re.search(r"loaded\s+memory-compare\.bin\s+at\s+0x0*1000", output, re.I):
        fail("binary load confirmation was not found", output)
    passed("binary loaded")

    if not re.search(r"\bstate\s*=\s*halted\b", output, re.I):
        fail("machine did not report halted state", output)
    passed("machine halted")

    if not re.search(r"\breason\s*=\s*HALT instruction\b", output, re.I):
        fail("HALT instruction was not the reported stop reason", output)
    passed("HALT instruction reached")

    require_register(output, 8, 0, "identical blocks compare equal")
    require_register(output, 9, 1, "first-byte difference is detected")
    require_register(output, 10, 1, "middle-byte difference is detected")
    require_register(output, 11, 1, "last-byte difference is detected")
    require_register(output, 12, 0, "zero-length comparison is equal")
    require_register(output, 14, 5, "all five test cases completed")
    require_register(output, 15, 0x0000F000, "stack pointer restored")
    require_register(output, 7, 1, "algorithm result is PASS")

    print("\nPASS: memory-compare")
    return 0

if __name__ == "__main__":
    sys.exit(main())
