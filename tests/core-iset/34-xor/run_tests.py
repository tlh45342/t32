#!/usr/bin/env python3
from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path

TEST_NAME = 'xor'
VM = os.environ.get("T32_RUN", "t32-run.exe" if os.name == "nt" else "t32-run")
CHECKS = [('binary loaded', 'loaded xor.bin at 0x00001000'), ('result', 'r2 =0x0000002a'), ('halted', 'state=halted'), ('instructions', 'instructions=4')]

def main() -> int:
    here = Path(__file__).resolve().parent
    script_path = here / "test.script"
    binary_path = here / f"{TEST_NAME}.bin"
    log_path = here / f"{TEST_NAME}.log"

    print(f"Running {TEST_NAME}...")

    if not binary_path.exists():
        print(f"FAIL missing binary: {binary_path.name}")
        return 1

    if log_path.exists():
        log_path.unlink()

    with script_path.open("r", encoding="utf-8") as script:
        completed = subprocess.run(
            [VM],
            cwd=here,
            stdin=script,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            check=False,
        )

    if completed.returncode != 0:
        print(completed.stdout)
        print(f"FAIL runner exit code {completed.returncode}")
        return 1

    if not log_path.exists():
        print(completed.stdout)
        print(f"FAIL missing log: {log_path.name}")
        return 1

    log = log_path.read_text(encoding="utf-8")
    passed = True

    for label, expected in CHECKS:
        if expected in log:
            print(f"  PASS {label}")
        else:
            print(f"  FAIL {label}")
            print(f"       missing: {expected}")
            passed = False

    print(f"{TEST_NAME}: {'PASS' if passed else 'FAIL'}")
    return 0 if passed else 1

if __name__ == "__main__":
    raise SystemExit(main())
