from pathlib import Path
import subprocess,sys
R=Path(__file__).resolve().parent
T=sorted(p for p in R.iterdir() if p.is_dir() and p.name[:2].isdigit())
print("Running T32 ABI 0.1 validation suite...\n")
for d in T:
 print("="*64); print(d.name); print("="*64)
 r=subprocess.run(["make","test"],cwd=d)
 if r.returncode: print(f"ABI suite: FAIL in {d.name}"); sys.exit(r.returncode)
 print()
print(f"T32 ABI 0.1: PASS ({len(T)}/{len(T)} tests)")
