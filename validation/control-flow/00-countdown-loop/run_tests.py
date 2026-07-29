#!/usr/bin/env python3
from pathlib import Path
import sys

HERE = Path(__file__).resolve()
ROOT = HERE.parents[3]
sys.path.insert(0, str(ROOT / "tools"))

from t32_validation import run_validation

raise SystemExit(run_validation(
    name='countdown-loop',
    checks=[('counter exhausted', 'r0 =0x00000000'), ('loop count accumulated', 'r1 =0x00000005'), ('halted', 'state=halted'), ('instruction count', 'instructions=18')],
    ordered=[('initial r0', 'r0 =0x00000005'), ('initial r1', 'r1 =0x00000000'), ('after first iteration r0', 'r0 =0x00000004'), ('after first iteration r1', 'r1 =0x00000001'), ('after second iteration r0', 'r0 =0x00000003'), ('after second iteration r1', 'r1 =0x00000002'), ('after third iteration r0', 'r0 =0x00000002'), ('after third iteration r1', 'r1 =0x00000003'), ('after fourth iteration r0', 'r0 =0x00000001'), ('after fourth iteration r1', 'r1 =0x00000004'), ('after fifth iteration r0', 'r0 =0x00000000'), ('after fifth iteration r1', 'r1 =0x00000005'), ('fall-through pc', 'pc =0x0000102c'), ('halt reached', 'state=halted')],
    forbidden=[
        ("no runtime error", "[ERROR]"),
        ("no fault state", "state=error"),
    ],
    counts=[],
))
