#!/usr/bin/env python3
from pathlib import Path
import sys

HERE = Path(__file__).resolve()
ROOT = HERE.parents[3]
sys.path.insert(0, str(ROOT / "tools"))

from t32_validation import run_validation

raise SystemExit(run_validation(
    name='simple-call',
    checks=[('result', 'r0 =0x0000002a'), ('return address bytes', '0x00002ffc: 18 10 00 00'), ('stack restored', 'r15=0x00003000'), ('halted', 'state=halted'), ('instruction count', 'instructions=6')],
    ordered=[('before call sp', 'r15=0x00003000'), ('before call pc', 'pc =0x00001010'), ('entered function sp', 'r15=0x00002ffc'), ('entered function pc', 'pc =0x0000101c'), ('saved return address', '0x00002ffc: 18 10 00 00'), ('function result', 'r0 =0x0000002a'), ('before ret sp', 'r15=0x00002ffc'), ('before ret pc', 'pc =0x00001024'), ('returned stack', 'r15=0x00003000'), ('returned pc', 'pc =0x00001018'), ('halted after return', 'state=halted')],
    forbidden=[
        ("runtime error", "[ERROR]"),
        ("fault state", "state=error"),
    ],
))
