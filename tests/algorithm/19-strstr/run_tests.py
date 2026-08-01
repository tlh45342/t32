from __future__ import annotations
import re
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
LOG = HERE / "strstr.log"


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
    print("Running strstr validation...\n")
    LOG.unlink(missing_ok=True)

    try:
        result = subprocess.run(
            ["t32-run"],
            input=(HERE / "test.script").read_text(encoding="utf-8"),
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            cwd=HERE,
            check=False,
        )
    except FileNotFoundError:
        fail("t32-run was not found in PATH")

    output = result.stdout or ""
    if LOG.exists():
        output += "\n" + LOG.read_text(encoding="utf-8", errors="replace")

    if result.returncode != 0:
        fail(f"t32-run exited with status {result.returncode}", output)
    passed("t32-run exited successfully")

    for pattern, label in (
        (r"loaded\s+strstr\.bin\s+at\s+0x0*1000", "binary loaded"),
        (r"\bstate\s*=\s*halted\b", "machine halted"),
        (r"\breason\s*=\s*HALT instruction\b", "HALT instruction reached"),
    ):
        if not re.search(pattern, output, re.I):
            fail(label, output)
        passed(label)

    require_register(output, 14, 8, "all eight test cases completed")
    require_register(output, 15, 0x0000F000, "stack pointer restored")
    require_register(output, 7, 1, "algorithm result is PASS")

    print("\nPASS: strstr")
    return 0


if __name__ == "__main__":
    sys.exit(main())
