#!/usr/bin/env python3
from __future__ import annotations
import os, subprocess
from pathlib import Path
TEST_NAME = 'ldb'
VM = os.environ.get("T32_RUN", "t32-run.exe" if os.name == "nt" else "t32-run")
CASES = [{'name': 'zero_extend_ff', 'steps': 5, 'checks': ['r0 =0x00003000', 'r1 =0xffffffff', 'r2 =0x000000ff'], 'encoding_checks': [(20, '00002010')]}, {'name': 'offset_bytes', 'steps': 8, 'checks': ['r2 =0x00000022'], 'encoding_checks': [(40, '00002010')]}, {'name': 'overwrite', 'steps': 7, 'checks': ['r2 =0x00000055'], 'encoding_checks': [(32, '00002010')]}, {'name': 'source_preserved', 'steps': 5, 'checks': ['r0 =0x00003000', 'r1 =0x0000007e', 'r2 =0x0000007e'], 'encoding_checks': [(20, '00002010')]}, {'name': 'flags_preserved', 'steps': 7, 'checks': ['r2 =0x0000005a', 'carry=1', 'zero=1', 'negative=0', 'overflow=0'], 'encoding_checks': [(36, '00002010')]}, {'name': 'unaligned_from_word', 'steps': 6, 'checks': ['r2 =0x000000cc'], 'encoding_checks': [(28, '00002010')]}]

def run_case(here: Path, case: dict[str, object]) -> bool:
    name=str(case["name"]); binary=f"{TEST_NAME}_{name}.bin"; log=f"{TEST_NAME}_{name}.log"
    bp=here/binary; lp=here/log
    print(f"  {name}")
    if not bp.exists(): print(f"    FAIL missing binary: {binary}"); return False
    data=bp.read_bytes()
    for check in case["encoding_checks"]:
        off=int(check[0]); exp=bytes.fromhex(str(check[1])); got=data[off:off+len(exp)]
        if got != exp:
            print(f"    FAIL assembler encoding at offset {off}")
            print(f"         expected: {exp.hex(' ')}")
            print(f"         actual:   {got.hex(' ')}")
            return False
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
