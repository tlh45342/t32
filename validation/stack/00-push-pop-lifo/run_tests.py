#!/usr/bin/env python3
from pathlib import Path
import sys

HERE = Path(__file__).resolve()
ROOT = HERE.parents[3]
sys.path.insert(0, str(ROOT / "tools"))

from t32_validation import run_validation

raise SystemExit(run_validation(
    name='push-pop-lifo',
    checks=[('last value popped first', 'r3 =0x0000000c'), ('middle value popped second', 'r4 =0x00000014'), ('first value popped last', 'r5 =0x0000000a'), ('stack restored', 'r15=0x00003000'), ('halted', 'state=halted'), ('instruction count', 'instructions=11'), ('stack memory contents', '0x00002ff4: 0c 00 00 00 14 00 00 00 0a 00 00 00')],
    ordered=[],
    forbidden=[
        ("runtime error", "[ERROR]"),
        ("fault state", "state=error"),
    ],
))
