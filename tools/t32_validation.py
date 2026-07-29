#!/usr/bin/env python3
from __future__ import annotations

import os
import subprocess
from pathlib import Path
from typing import Sequence


def run_validation(
    *,
    name: str,
    checks: Sequence[tuple[str, str]] = (),
    ordered: Sequence[tuple[str, str]] = (),
    forbidden: Sequence[tuple[str, str]] = (),
    counts: Sequence[tuple[str, str, int]] = (),
    timeout_seconds: int = 10,
) -> int:
    here = Path.cwd()
    vm = os.environ.get(
        "T32_RUN",
        "t32-run.exe" if os.name == "nt" else "t32-run",
    )

    script_path = here / "test.script"
    binary_path = here / f"{name}.bin"
    log_path = here / f"{name}.log"

    print(f"Running validation {name}...")

    for required in (script_path, binary_path):
        if not required.exists():
            print(f"FAIL missing file: {required.name}")
            return 1

    if log_path.exists():
        log_path.unlink()

    try:
        with script_path.open("r", encoding="utf-8") as script:
            completed = subprocess.run(
                [vm],
                cwd=here,
                stdin=script,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                timeout=timeout_seconds,
                check=False,
            )
    except FileNotFoundError:
        print(f"FAIL runner not found: {vm}")
        return 1
    except subprocess.TimeoutExpired:
        print(f"FAIL runner timed out after {timeout_seconds} seconds")
        return 1

    if completed.returncode != 0:
        print(completed.stdout)
        print(f"FAIL runner exit code: {completed.returncode}")
        return 1

    if not log_path.exists():
        print(completed.stdout)
        print(f"FAIL missing log: {log_path.name}")
        return 1

    log = log_path.read_text(encoding="utf-8")
    passed = True

    for label, expected in checks:
        if expected in log:
            print(f"  PASS {label}")
        else:
            print(f"  FAIL {label}")
            print(f"       missing: {expected}")
            passed = False

    cursor = 0
    for label, expected in ordered:
        pos = log.find(expected, cursor)
        if pos < 0:
            print(f"  FAIL ordered checkpoint: {label}")
            print(f"       missing after offset {cursor}: {expected}")
            passed = False
        else:
            print(f"  PASS ordered checkpoint: {label}")
            cursor = pos + len(expected)

    for label, text in forbidden:
        if text in log:
            print(f"  FAIL {label}")
            print(f"       forbidden text present: {text}")
            passed = False
        else:
            print(f"  PASS {label}")

    for label, text, expected_count in counts:
        actual = log.count(text)
        if actual == expected_count:
            print(f"  PASS count {label}: {actual}")
        else:
            print(f"  FAIL count {label}: expected {expected_count}, got {actual}")
            passed = False

    print(f"{name}: {'PASS' if passed else 'FAIL'}")
    return 0 if passed else 1
