from __future__ import annotations
import re, subprocess, sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
LOG = HERE / "strcmp.log"

def passed(msg): print(f"PASS {msg}")

def fail(msg, output=""):
    print(f"FAIL {msg}")
    if output:
        print("\n--- t32-run output ---")
        print(output.rstrip())
        print("--- end output ---")
    raise SystemExit(1)

def reg(text, n):
    m = re.search(rf"\br{n}\s*=\s*(0x[0-9a-fA-F]+|\d+)\b", text)
    return int(m.group(1), 0) if m else None

def expect(text, n, value, label):
    actual = reg(text, n)
    if actual != value:
        fail(f"{label}: expected r{n}=0x{value:08X}, got {actual!r}", text)
    passed(label)

def main():
    print("Running strcmp validation...\n")
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

    output = (result.stdout or "")
    if LOG.exists():
        output += "\n" + LOG.read_text(encoding="utf-8", errors="replace")

    if result.returncode != 0:
        fail(f"t32-run exited with status {result.returncode}", output)
    passed("t32-run exited successfully")

    for pattern, label in [
        (r"loaded\s+strcmp\.bin\s+at\s+0x0*1000", "binary loaded"),
        (r"\bstate\s*=\s*halted\b", "machine halted"),
        (r"\breason\s*=\s*HALT instruction\b", "HALT instruction reached"),
    ]:
        if not re.search(pattern, output, re.I):
            fail(label, output)
        passed(label)

    expect(output, 8, 0, "empty strings compare equal")
    expect(output, 9, 0, "identical strings compare equal")
    expect(output, 10, 0xFFFFFFFF, "left string sorts before right")
    expect(output, 11, 1, "left string sorts after right")
    expect(output, 12, 0xFFFFFF9D, "shorter left string sorts before")
    expect(output, 13, 99, "longer left string sorts after")
    expect(output, 14, 8, "all eight test cases completed")
    expect(output, 15, 0x0000F000, "stack pointer restored")
    expect(output, 7, 1, "algorithm result is PASS")

    print("\nPASS: strcmp")
    return 0

if __name__ == "__main__":
    sys.exit(main())
