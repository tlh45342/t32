#!/usr/bin/env python3
from __future__ import annotations

import os
import subprocess
from pathlib import Path

TEST_NAME = 'cmp'
VM = os.environ.get("T32_RUN", "t32-run.exe" if os.name == "nt" else "t32-run")
CASES = [{'name': 'equal', 'steps': 4, 'encoding_offset': 16, 'encoding_hex': '00100028', 'checks': ['r0 =0x0000002a', 'r1 =0x0000002a', 'carry=1', 'zero=1', 'negative=0', 'overflow=0']}, {'name': 'greater', 'steps': 4, 'encoding_offset': 16, 'encoding_hex': '00100028', 'checks': ['r0 =0x0000002a', 'r1 =0x00000014', 'carry=1', 'zero=0', 'negative=0', 'overflow=0']}, {'name': 'less', 'steps': 4, 'encoding_offset': 16, 'encoding_hex': '00100028', 'checks': ['r0 =0x00000003', 'r1 =0x00000005', 'carry=0', 'zero=0', 'negative=1', 'overflow=0']}, {'name': 'signed_overflow', 'steps': 4, 'encoding_offset': 16, 'encoding_hex': '00100028', 'checks': ['r0 =0x80000000', 'r1 =0x00000001', 'carry=1', 'zero=0', 'negative=0', 'overflow=1']}, {'name': 'negative_equal', 'steps': 4, 'encoding_offset': 16, 'encoding_hex': '00100028', 'checks': ['r0 =0xffffffff', 'r1 =0xffffffff', 'carry=1', 'zero=1', 'negative=0', 'overflow=0']}]


def run_case(here: Path, case: dict[str, object]) -> bool:
    case_name = str(case["name"])
    binary_name = f"{TEST_NAME}_{case_name}.bin"
    log_name = f"{TEST_NAME}_{case_name}.log"
    binary_path = here / binary_name
    log_path = here / log_name

    print(f"  {case_name}")
    if not binary_path.exists():
        print(f"    FAIL missing binary: {binary_name}")
        return False

    data = binary_path.read_bytes()
    offset = int(case["encoding_offset"])
    expected_encoding = bytes.fromhex(str(case["encoding_hex"]))
    actual_encoding = data[offset:offset + len(expected_encoding)]
    if actual_encoding != expected_encoding:
        print("    FAIL assembler encoding")
        print(f"         expected: {expected_encoding.hex(' ')}")
        print(f"         actual:   {actual_encoding.hex(' ')}")
        return False
    print("    PASS assembler encoding")

    if log_path.exists():
        log_path.unlink()

    steps = int(case["steps"])
    dump_size = len(data)
    script = "\n".join([
        f"logfile {log_name}",
        "version",
        "reset",
        f"load {binary_name} 0x1000",
        f"e 0x1000 {dump_size}",
        "set pc 0x1000",
        f"set run steps {steps}",
        "run",
        "regs",
        "status",
        "logfile off",
        "",
    ])

    completed = subprocess.run(
        [VM], cwd=here, input=script, stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT, text=True, check=False,
    )
    if completed.returncode != 0:
        print(completed.stdout)
        print(f"    FAIL runner exit code {completed.returncode}")
        return False
    if not log_path.exists():
        print(completed.stdout)
        print(f"    FAIL missing log: {log_name}")
        return False

    log = log_path.read_text(encoding="utf-8")
    expected = [
        f"loaded {binary_name} at 0x00001000",
        "state=halted",
        f"instructions={steps}",
        *case["checks"],
    ]
    passed = True
    for value in expected:
        if str(value) in log:
            print(f"    PASS {value}")
        else:
            print(f"    FAIL missing: {value}")
            passed = False
    return passed


def main() -> int:
    here = Path(__file__).resolve().parent
    print(f"Running {TEST_NAME} expanded validation...")
    results = [run_case(here, case) for case in CASES]
    passed = all(results)
    print(f"{TEST_NAME}: {'PASS' if passed else 'FAIL'} ({sum(results)}/{len(results)} cases)")
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
