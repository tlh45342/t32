from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
SCRIPT = HERE / "test.script"
LOG = HERE / "memmove.log"


def passed(message: str) -> None:
    print(f"PASS {message}")


def fail(message: str, output: str = "") -> None:
    print(f"FAIL {message}")
    if output:
        print("\n--- t32-run output ---")
        print(output.rstrip())
        print("--- end output ---")
    raise SystemExit(1)


def register_value(text: str, register: int) -> int | None:
    match = re.search(
        rf"\br{register}\s*=\s*(0x[0-9a-fA-F]+|\d+)\b",
        text,
    )
    return int(match.group(1), 0) if match else None


def require_register(text: str, register: int, expected: int, label: str) -> None:
    actual = register_value(text, register)
    if actual is None:
        fail(f"r{register} was not reported while checking {label}", text)
    if actual != expected:
        fail(
            f"{label}: expected r{register}=0x{expected:08X}, "
            f"got 0x{actual:08X}",
            text,
        )
    passed(label)


def main() -> int:
    print("Running memmove validation...\n")
    LOG.unlink(missing_ok=True)

    try:
        script_text = SCRIPT.read_text(encoding="utf-8")
        result = subprocess.run(
            ["t32-run"],
            input=script_text,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            cwd=HERE,
            check=False,
        )
    except FileNotFoundError:
        fail("t32-run was not found in PATH")
    except OSError as exc:
        fail(str(exc))

    console = result.stdout or ""
    log_text = (
        LOG.read_text(encoding="utf-8", errors="replace")
        if LOG.exists()
        else ""
    )
    output = console + "\n" + log_text

    if result.returncode != 0:
        fail(f"t32-run exited with status {result.returncode}", output)
    passed("t32-run exited successfully")

    if not re.search(r"loaded\s+memmove\.bin\s+at\s+0x0*1000", output, re.I):
        fail("binary load confirmation was not found", output)
    passed("binary loaded")

    if not re.search(r"\bstate\s*=\s*halted\b", output, re.I):
        fail("machine did not report halted state", output)
    passed("machine halted")

    if not re.search(r"\breason\s*=\s*HALT instruction\b", output, re.I):
        fail("HALT instruction was not the reported stop reason", output)
    passed("HALT instruction reached")

    require_register(output, 14, 6, "all six test cases completed")
    require_register(output, 15, 0x0000F000, "stack pointer restored")
    require_register(output, 7, 1, "algorithm result is PASS")

    print("\nPASS: memmove")
    return 0


if __name__ == "__main__":
    sys.exit(main())
