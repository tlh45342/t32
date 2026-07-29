#!/usr/bin/env python3
from pathlib import Path
import sys

HERE = Path(__file__).resolve()
ROOT = HERE.parents[3]
sys.path.insert(0, str(ROOT / "tools"))

from t32_validation import run_validation

raise SystemExit(run_validation(
    name='stack-memory-layout',
    checks=[('first push bytes', '0x00002ffc: 44 33 22 11'), ('two-value stack layout', '0x00002ff8: 88 77 66 55 44 33 22 11'), ('final halted state', 'state=halted'), ('instruction count', 'instructions=6')],
    ordered=[('before pushes sp', 'r15=0x00003000'), ('before pushes pc', 'pc =0x00001018'), ('after first push sp', 'r15=0x00002ffc'), ('after first push pc', 'pc =0x0000101c'), ('first pushed value', '0x00002ffc: 44 33 22 11'), ('after second push sp', 'r15=0x00002ff8'), ('after second push pc', 'pc =0x00001020'), ('two values', '0x00002ff8: 88 77 66 55 44 33 22 11'), ('halt reached', 'state=halted')],
    forbidden=[
        ("runtime error", "[ERROR]"),
        ("fault state", "state=error"),
    ],
))
