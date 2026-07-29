#!/usr/bin/env python3
from pathlib import Path
import sys

HERE = Path(__file__).resolve()
ROOT = HERE.parents[3]
sys.path.insert(0, str(ROOT / "tools"))

from t32_validation import run_validation

raise SystemExit(run_validation(
    name='backward-jmp',
    checks=[('three loop iterations', 'r0 =0x00000003'), ('loop terminator zero', 'r1 =0x00000000'), ('halted', 'state=halted'), ('instructions', 'instructions=15')],
    ordered=[],
    forbidden=[
        ("no runtime error", "[ERROR]"),
        ("no fault state", "state=error"),
    ],
    counts=[],
))
