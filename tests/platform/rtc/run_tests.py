from pathlib import Path
import re, subprocess, sys

def need(ok, name, out=''):
    if not ok:
        print(f'  FAIL {name}')
        if out: print(out)
        raise SystemExit(1)
    print(f'  PASS {name}')

def reg(out, n):
    m=re.search(rf'\br{n}\s*=\s*0x([0-9a-fA-F]+)',out)
    return int(m.group(1),16) if m else None

def main():
    runner=Path(sys.argv[1]).resolve(); image=Path(sys.argv[2]).resolve()
    print('Running T32 RTC platform validation...')
    need(runner.is_file(),'t32-run exists')
    need(image.is_file(),'rtc.bin exists')
    script='\n'.join([f'load {image} 0x1000','set pc 0x1000','run','regs','status','quit',''])
    p=subprocess.run([str(runner)],input=script,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    out=p.stdout or ''
    need(p.returncode==0,'RTC guest executes',out)
    need(reg(out,4)==0x54335231,'RTC ID visible to guest',out)
    s=reg(out,5); a=reg(out,6); b=reg(out,7)
    need(s is not None and (s&1)==1,'RTC STATUS valid bit visible',out)
    need(a is not None and a>1700000000,'RTC EPOCH is plausible',out)
    need(b is not None and b>=a,'RTC EPOCH does not move backward',out)
    need('state=halted' in out.lower(),'RTC test halts cleanly',out)
    print('rtc: PASS (7/7 cases)')
    return 0

if __name__=='__main__': raise SystemExit(main())
