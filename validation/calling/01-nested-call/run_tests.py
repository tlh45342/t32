#!/usr/bin/env python3
from pathlib import Path
import sys

HERE = Path(__file__).resolve()
ROOT = HERE.parents[3]
sys.path.insert(0, str(ROOT / "tools"))

from t32_validation import run_validation

raise SystemExit(run_validation(
    name='nested-call',
    checks=[('final result', 'r0 =0x0000002a'), ('outer return address', '0x00002ffc: 18 10 00 00'), ('nested return addresses', '0x00002ff8: 2c 10 00 00 18 10 00 00'), ('stack restored', 'r15=0x00003000'), ('halted', 'state=halted'), ('instruction count', 'instructions=10')],
    ordered=[('before outer call sp', 'r15=0x00003000'), ('before outer call pc', 'pc =0x00001010'), ('outer frame sp', 'r15=0x00002ffc'), ('outer function pc', 'pc =0x0000101c'), ('outer return stored', '0x00002ffc: 18 10 00 00'), ('outer arithmetic', 'r0 =0x0000001e'), ('inner call location', 'pc =0x00001024'), ('inner frame sp', 'r15=0x00002ff8'), ('inner function pc', 'pc =0x00001038'), ('both return addresses', '0x00002ff8: 2c 10 00 00 18 10 00 00'), ('inner arithmetic', 'r0 =0x00000025'), ('inner ret frame restored', 'r15=0x00002ffc'), ('inner ret target', 'pc =0x0000102c'), ('outer completion', 'r0 =0x0000002a'), ('outer ret pending sp', 'r15=0x00002ffc'), ('outer ret instruction pc', 'pc =0x00001034'), ('final stack restored', 'r15=0x00003000'), ('outer ret target', 'pc =0x00001018'), ('final halt', 'state=halted')],
    forbidden=[
        ("runtime error", "[ERROR]"),
        ("fault state", "state=error"),
    ],
))
