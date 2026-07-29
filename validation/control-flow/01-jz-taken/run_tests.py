#!/usr/bin/env python3
from pathlib import Path
import sys

HERE = Path(__file__).resolve()
ROOT = HERE.parents[3]
sys.path.insert(0, str(ROOT / "tools"))

from t32_validation import run_validation

raise SystemExit(run_validation(
    name='jz-taken',
    checks=[('taken value', 'r1 =0x0000002a'), ('halted', 'state=halted'), ('instructions', 'instructions=4')],
    ordered=[('zero source established', 'r0 =0x00000000'), ('before branch pc', 'pc =0x00001008'), ('branch target pc', 'pc =0x00001018'), ('target executed', 'r1 =0x0000002a'), ('halt reached', 'state=halted')],
    forbidden=[
        ("no runtime error", "[ERROR]"),
        ("no fault state", "state=error"),
    ],
    counts=[],
))
