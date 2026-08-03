#!/usr/bin/env python3
from __future__ import annotations
import os, re, shutil, subprocess, sys
from pathlib import Path
ROOT=Path(__file__).resolve().parent; CASES=ROOT/'cases'; EXPECTED=ROOT/'expected'; BUILD=ROOT/'build'

def run(cmd, env=None, input_text=None):
    return subprocess.run(cmd,input=input_text,text=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE,check=False,env=env)
def req(c,n,d=''):
    if c: print(f'  PASS {n}'); return
    print(f'  FAIL {n}'); print(d.rstrip() if d else ''); raise SystemExit(1)
def reg(text,n):
    m=re.search(rf'\br{n}\s*=\s*(0x[0-9a-fA-F]+|\d+)\b',text); return int(m.group(1),0) if m else None

def execute(binary):
    script='\n'.join(['reset',f'load {binary} 0x00001000','set pc 0x00001000','run','regs','status','quit',''])
    ex=run(['t32-run'],input_text=script)
    return ex,ex.stdout+ex.stderr

def main():
    cc=Path(sys.argv[1] if len(sys.argv)>1 else './t32-cc').resolve(); req(cc.is_file(),'compiler exists',str(cc))
    shutil.rmtree(BUILD,ignore_errors=True); BUILD.mkdir(parents=True)
    print('Running t32-cc Stage 3 local-variable tests...')
    v=run([str(cc),'--version']); req(v.returncode==0,'--version exits successfully',v.stderr); req('t32-cc 0.2.0' in v.stdout,'--version reports 0.2.0',v.stdout)

    # Preserve Stage 2 constant-return behavior.
    s=BUILD/'return_42.s'; r=run([str(cc),'-S',str(CASES/'return_42.c'),'-o',str(s)]); req(r.returncode==0,'constant -S compile succeeds',r.stderr); req(r.stdout=='','quiet by default',r.stdout); req(s.read_text()==(EXPECTED/'return_42.s').read_text(),'constant ABI assembly matches',s.read_text())

    # First typed local variable and exact unoptimized stack code.
    ls=BUILD/'local_x.s'; r=run([str(cc),'-S',str(CASES/'local_x.c'),'-o',str(ls)]); req(r.returncode==0,'local variable -S compile succeeds',r.stderr); req(ls.read_text()==(EXPECTED/'local_x.s').read_text(),'local stack assembly matches',ls.read_text())
    text=ls.read_text(); req('subi r15, r15, 4' in text,'local allocates four-byte stack slot'); req('stw  r1, [r15]' in text,'initializer stored to local slot'); req('ldw  r0, [r15]' in text,'local loaded into return register'); req('addi r15, r15, 4' in text,'local stack slot released')

    # Object and full execution for local x = 5.
    obj=BUILD/'local_x.o'; r=run([str(cc),'-c',str(CASES/'local_x.c'),'-o',str(obj)]); req(r.returncode==0 and obj.exists(),'-c emits local-variable object',r.stderr)
    nm=run(['t32-nm',str(obj)]); req(nm.returncode==0 and re.search(r'\bmain\b',nm.stdout),'local object exports main',nm.stdout+nm.stderr)
    binary=BUILD/'local_x.bin'; r=run([str(cc),str(CASES/'local_x.c'),'-o',str(binary)]); req(r.returncode==0 and binary.exists(),'local program links',r.stderr)
    ex,allout=execute(binary); req(ex.returncode==0,'local program executes',allout); req(reg(allout,0)==5,'local main returns 5',allout); req(reg(allout,15)==0xF000,'local program restores stack',allout); req(re.search(r'state\s*=\s*halted',allout,re.I) is not None,'local program halts',allout)

    # Signed initializer proves values pass through memory unchanged.
    neg=BUILD/'local_negative.bin'; r=run([str(cc),str(CASES/'local_negative.c'),'-o',str(neg)]); req(r.returncode==0 and neg.exists(),'negative local program links',r.stderr)
    ex,allout=execute(neg); req(reg(allout,0)==0xFFFFFFF9,'negative local returns -7 bit pattern',allout); req(reg(allout,15)==0xF000,'negative local restores stack',allout)

    # Verbose driver remains opt-in.
    vr=run([str(cc),'-v','-c',str(CASES/'local_x.c'),'-o',str(BUILD/'verbose.o')]); req(vr.returncode==0 and 'invoke: t32-as' in vr.stdout,'-v shows invoked phase',vr.stdout+vr.stderr)

    # Semantic and syntax negatives.
    negatives=[
      ('undeclared_local.c','undeclared local variable','undeclared local'),
      ('two_locals.c','exactly one local variable','second local'),
      ('local_missing_initializer.c',"expected '='",'missing initializer'),
      ('invalid_expression.c',("integer literal or local variable","expected ';'","found '+'"),'unsupported expression'),
      ('invalid_function.c',"expected 'main'",'non-main function'),
      ('invalid_missing_semicolon.c',"expected ';'",'missing semicolon'),
    ]
    for case,needle,label in negatives:
        dest=BUILD/(label.replace(' ','_')+'.s'); rr=run([str(cc),'-S',str(CASES/case),'-o',str(dest)]); req(rr.returncode!=0,f'reject {label}');
        if isinstance(needle, tuple):
            diagnostic_ok = (needle[0] in rr.stderr) or all(part in rr.stderr for part in needle[1:])
        else:
            diagnostic_ok = needle in rr.stderr
        req(diagnostic_ok,f'{label} diagnostic',rr.stderr); req(not dest.exists(),f'{label} leaves no output')

    bad=run([str(cc),'-S','-c',str(CASES/'return_0.c')]); req(bad.returncode!=0,'reject -S with -c'); req('cannot be used together' in bad.stderr,'mode conflict diagnostic',bad.stderr)
    missing=BUILD/'missing.bin'; env=os.environ.copy(); env['T32_PREFIX']=str(BUILD/'no-runtime'); rr=run([str(cc),str(CASES/'return_0.c'),'-o',str(missing)],env=env); req(rr.returncode!=0,'missing runtime rejected'); req('missing startup object' in rr.stderr,'missing runtime diagnostic',rr.stderr); req(not missing.exists(),'failed link leaves no binary')
    absent=run([str(cc),'-S',str(CASES/'does-not-exist.c')]); req(absent.returncode!=0,'missing source rejected'); req('cannot open' in absent.stderr,'missing source diagnostic',absent.stderr)
    print('t32-cc: PASS (48/48 cases)'); return 0
if __name__=='__main__': raise SystemExit(main())
