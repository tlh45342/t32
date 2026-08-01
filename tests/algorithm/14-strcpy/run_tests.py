from __future__ import annotations
import re, subprocess, sys
from pathlib import Path
HERE=Path(__file__).resolve().parent
SCRIPT=HERE/"test.script"
LOG=HERE/"strcpy.log"
def passed(m): print(f"PASS {m}")
def fail(m,o=""):
 print(f"FAIL {m}")
 if o:
  print("\n--- t32-run output ---")
  print(o.rstrip())
  print("--- end output ---")
 raise SystemExit(1)
def reg(text,n):
 m=re.search(rf"\br{n}\s*=\s*(0x[0-9a-fA-F]+|\d+)\b",text)
 return int(m.group(1),0) if m else None
def require(text,n,e,label):
 a=reg(text,n)
 if a is None: fail(f"r{n} was not reported while checking {label}",text)
 if a!=e: fail(f"{label}: expected r{n}=0x{e:08X}, got 0x{a:08X}",text)
 passed(label)
def main():
 print("Running strcpy validation...\n")
 LOG.unlink(missing_ok=True)
 try:
  r=subprocess.run(["t32-run"],input=SCRIPT.read_text(),text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,cwd=HERE,check=False)
 except FileNotFoundError: fail("t32-run was not found in PATH")
 out=(r.stdout or "")+"\n"+(LOG.read_text(encoding="utf-8",errors="replace") if LOG.exists() else "")
 if r.returncode!=0: fail(f"t32-run exited with status {r.returncode}",out)
 passed("t32-run exited successfully")
 if not re.search(r"loaded\s+strcpy\.bin\s+at\s+0x0*1000",out,re.I): fail("binary load confirmation was not found",out)
 passed("binary loaded")
 if not re.search(r"\bstate\s*=\s*halted\b",out,re.I): fail("machine did not report halted state",out)
 passed("machine halted")
 if not re.search(r"\breason\s*=\s*HALT instruction\b",out,re.I): fail("HALT instruction was not the reported stop reason",out)
 passed("HALT instruction reached")
 require(out,14,5,"all five test cases completed")
 require(out,15,0xF000,"stack pointer restored")
 require(out,7,1,"algorithm result is PASS")
 print("\nPASS: strcpy")
 return 0
if __name__=="__main__": sys.exit(main())
