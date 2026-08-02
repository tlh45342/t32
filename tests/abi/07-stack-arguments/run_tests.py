from __future__ import annotations
import re,subprocess,sys
from pathlib import Path
HERE=Path(__file__).resolve().parent; NAME=HERE.name; STEM=NAME.split('-',1)[1]; LOG=HERE/(STEM+'.log')
def ok(m): print(f"PASS {m}")
def fail(m,o=''):
 print(f"FAIL {m}")
 if o: print("\n--- t32-run output ---\n"+o.rstrip()+"\n--- end output ---")
 raise SystemExit(1)
def reg(t,n):
 m=re.search(rf"\br{n}\s*=\s*(0x[0-9a-fA-F]+|\d+)\b",t); return int(m.group(1),0) if m else None
def expect(t,n,v,label):
 a=reg(t,n)
 if a!=v: fail(f"{label}: expected r{n}=0x{v:08X}, got {a!r}",t)
 ok(label)
def main():
 print(f"Running ABI validation: {NAME}...\n"); LOG.unlink(missing_ok=True)
 try:
  r=subprocess.run(['t32-run'],input=(HERE/'test.script').read_text(),text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,cwd=HERE,check=False)
 except FileNotFoundError: fail('t32-run was not found in PATH')
 out=r.stdout or ''
 if LOG.exists(): out+='\n'+LOG.read_text(errors='replace')
 if r.returncode: fail(f't32-run exited with status {r.returncode}',out)
 ok('t32-run exited successfully')
 for p,l in [(rf"loaded\s+{re.escape(STEM+'.bin')}\s+at\s+0x0*1000",'binary loaded'),(r'\bstate\s*=\s*halted\b','machine halted'),(r'\breason\s*=\s*HALT instruction\b','HALT instruction reached')]:
  if not re.search(p,out,re.I): fail(l,out)
  ok(l)
 expect(out,7,1,'ABI result is PASS'); expect(out,15,0xF000,'stack pointer restored')
 print(f"\nPASS: {NAME}"); return 0
if __name__=='__main__': sys.exit(main())
