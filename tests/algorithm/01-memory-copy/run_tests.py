#!/usr/bin/env python3

from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
BINARY = ROOT / "memory-copy.bin"
SCRIPT = ROOT / "test.script"
LOG = ROOT / "memory-copy.log"

def initialize():
    for stream in (sys.stdin, sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8")
        except Exception:
            pass

def check(label: str, condition: bool) -> bool:
    marker = "✅" if condition else "❌"
    print(f"  {marker} {label}")
    return condition


def main() -> int:
    initialize()
    print("Running memory-copy validation...")

    if LOG.exists():
        LOG.unlink()

    try:
        result = subprocess.run(
            ["t32-run"],
            cwd=ROOT,
            stdin=SCRIPT.open("r", encoding="utf-8"),
            text=True,
            capture_output=True,
            check=False,
        )
    except OSError as exc:
        print(f"ERROR: unable to run t32-run: {exc}")
        return 1

    output = result.stdout + result.stderr

    # Preserve the VM output for diagnosis even if t32-run did not create
    # the requested logfile.
    if not LOG.exists():
        LOG.write_text(output, encoding="utf-8")

    passed = True

    passed &= check("t32-run exited successfully", result.returncode == 0)
    passed &= check("machine halted", "state=halted" in output)
    passed &= check("HALT instruction reached", "reason=HALT instruction" in output)

    r7_match = re.search(r"\br7\s*=\s*0x([0-9a-fA-F]{8})", output)
    passed &= check("r7 was reported", r7_match is not None)

    if r7_match:
        r7 = int(r7_match.group(1), 16)
        passed &= check("algorithm result is PASS", r7 == 0)

    r1_match = re.search(r"\br1\s*=\s*0x([0-9a-fA-F]{8})", output)
    r2_match = re.search(r"\br2\s*=\s*0x([0-9a-fA-F]{8})", output)
    r3_match = re.search(r"\br3\s*=\s*0x([0-9a-fA-F]{8})", output)
    r4_match = re.search(r"\br4\s*=\s*0x([0-9a-fA-F]{8})", output)
    r5_match = re.search(r"\br5\s*=\s*0x([0-9a-fA-F]{8})", output)
    r6_match = re.search(r"\br6\s*=\s*0x([0-9a-fA-F]{8})", output)

    passed &= check(
        "source pointer advanced 16 bytes",
        r1_match is not None and int(r1_match.group(1), 16) == 0x00009010,
    )

    passed &= check(
        "destination pointer advanced 16 bytes",
        r2_match is not None and int(r2_match.group(1), 16) == 0x00009110,
    )

    passed &= check(
        "copy count reached zero",
        r3_match is not None and int(r3_match.group(1), 16) == 0,
    )

    passed &= check(
        "final source byte is 0xFF",
        r4_match is not None and int(r4_match.group(1), 16) == 0xFF,
    )

    passed &= check(
        "final destination byte is 0xFF",
        r5_match is not None and int(r5_match.group(1), 16) == 0xFF,
    )

    passed &= check(
        "final byte comparison matched",
        r6_match is not None and int(r6_match.group(1), 16) == 0,
    )

    print()
    if passed:
        print("PASS: memory-copy")
        return 0

    print("FAIL: memory-copy")
    print(f"See: {LOG}")
    return 1


if __name__ == "__main__":
    sys.exit(main())