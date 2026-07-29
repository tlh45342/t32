#!/usr/bin/env python3
from pathlib import Path
import sys

HERE = Path(__file__).resolve()
ROOT = HERE.parents[3]
sys.path.insert(0, str(ROOT / "tools"))

from t32_validation import run_validation

raise SystemExit(run_validation(
    name='jnz-not-taken',
    checks=[('fall-through value', 'r1 =0x0000002a'), ('halted', 'state=halted'), ('instructions', 'instructions=5')],
    ordered=[('zero source established', 'r0 =0x00000000'), ('before branch pc', 'pc =0x00001008'), ('fall-through pc', 'pc =0x00001010'), ('fall-through instruction executed', 'r1 =0x0000002a'), ('jump to done', 'pc =0x00001028'), ('halt reached', 'state=halted')],
    forbidden=[
        ("no runtime error", "[ERROR]"),
        ("no fault state", "state=error"),
    ],
    counts=[],
))
