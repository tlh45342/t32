#!/usr/bin/env python3
from __future__ import annotations
import os, subprocess
from pathlib import Path
TEST_NAME = 'sar'
VM = os.environ.get("T32_RUN", "t32-run.exe" if os.name == "nt" else "t32-run")
CASES = [{'name': 'negative', 'steps': 4, 'encoding_offset': 16, 'encoding_hex': '00102026', 'checks': ['r2 =0xffffffd6', 'negative=1', 'zero=0', 'carry=0', 'overflow=0']}, {'name': 'positive', 'steps': 4, 'encoding_offset': 16, 'encoding_hex': '00102026', 'checks': ['r2 =0x0000002a', 'negative=0', 'zero=0']}, {'name': 'zero_count', 'steps': 4, 'encoding_offset': 16, 'encoding_hex': '00102026', 'checks': ['r2 =0x80000001', 'negative=1', 'zero=0']}, {'name': 'by_31_negative', 'steps': 4, 'encoding_offset': 16, 'encoding_hex': '00102026', 'checks': ['r2 =0xffffffff', 'negative=1', 'zero=0', 'carry=0']}, {'name': 'by_31_positive', 'steps': 4, 'encoding_offset': 16, 'encoding_hex': '00102026', 'checks': ['r2 =0x00000000', 'negative=0', 'zero=1', 'carry=1']}, {'name': 'carry_out', 'steps': 4, 'encoding_offset': 16, 'encoding_hex': '00102026', 'checks': ['r2 =0xfffffffe', 'carry=1', 'negative=1', 'zero=0']}, {'name': 'alias', 'steps': 4, 'encoding_offset': 16, 'encoding_hex': '00100026', 'checks': ['r0 =0xffffffd6', 'r1 =0x00000001']}]

def run_case(here: Path, case: dict[str, object]) -> bool:
    name=str(case["name"]); binary=f"{TEST_NAME}_{name}.bin"; log=f"{TEST_NAME}_{name}.log"
    bp=here/binary; lp=here/log
    print(f"  {name}")
    if not bp.exists(): print(f"    FAIL missing binary: {binary}"); return False
    data=bp.read_bytes(); off=int(case["encoding_offset"]); exp=bytes.fromhex(str(case["encoding_hex"])); got=data[off:off+len(exp)]
    if got != exp:
        print("    FAIL assembler encoding"); print(f"         expected: {exp.hex(' ')}"); print(f"         actual:   {got.hex(' ')}"); return False
    print("    PASS assembler encoding")
    if lp.exists(): lp.unlink()
    steps=int(case["steps"])
    script="\n".join([f"logfile {log}","version","reset",f"load {binary} 0x1000",f"e 0x1000 {len(data)}","set pc 0x1000",f"set run steps {steps}","run","regs","status","logfile off",""])
    p=subprocess.run([VM],cwd=here,input=script,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,check=False)
    if p.returncode != 0: print(p.stdout); print(f"    FAIL runner exit code {p.returncode}"); return False
    if not lp.exists(): print(p.stdout); print(f"    FAIL missing log: {log}"); return False
    text=lp.read_text(encoding="utf-8")
    expected=[f"loaded {binary} at 0x00001000","state=halted",f"instructions={steps}",*case["checks"]]
    ok=True
    for value in expected:
        if str(value) in text: print(f"    PASS {value}")
        else: print(f"    FAIL missing: {value}"); ok=False
    return ok

def main()->int:
    here=Path(__file__).resolve().parent
    print(f"Running {TEST_NAME} expanded validation...")
    results=[run_case(here,c) for c in CASES]
    passed=all(results)
    print(f"{TEST_NAME}: {'PASS' if passed else 'FAIL'} ({sum(results)}/{len(results)} cases)")
    return 0 if passed else 1
if __name__=='__main__': raise SystemExit(main())
