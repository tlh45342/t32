from __future__ import annotations
import re, subprocess, sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
BUILD=ROOT/'build'; ARCHIVE=BUILD/'libt32.a'
def passed(m): print(f'  PASS {m}')
def fail(m,o=''):
 print(f'  FAIL {m}')
 if o: print('\n--- output ---\n'+o.rstrip()+'\n--- end output ---')
 raise SystemExit(1)
def run(c): return subprocess.run(c,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,check=False)
def run_case(name, expected, required, forbidden):
 case=ROOT/'tests'/name; stem=name.split('-',1)[-1]; out=BUILD/'tests'; out.mkdir(parents=True,exist_ok=True)
 main_o=out/f'{stem}.o'; program=out/f'{stem}.bin'; mapf=out/f'{stem}.map'; log=ROOT/f'{stem}.log'; log.unlink(missing_ok=True)
 r=run(['t32-as','-f','obj',str(case/'main.s'),'-o',str(main_o)])
 if r.returncode: fail(f'{stem} main module assembled',r.stdout)
 passed(f'{stem} main module assembled')
 r=run(['t32-ld','-Ttext','0x00001000',str(main_o),str(ARCHIVE),'-Map',str(mapf),'-o',str(program)])
 if r.returncode: fail(f'{stem} linked through libt32.a',r.stdout)
 passed(f'{stem} linked through libt32.a')
 mt=mapf.read_text(encoding='utf-8',errors='replace')
 for s in required:
  if s not in mt: fail(f'{stem} map contains {s}',mt)
 passed(f'{stem} required archive members selected')
 for s in forbidden:
  if re.search(rf'\b{re.escape(s)}\b',mt): fail(f'{stem} unused symbol {s} was extracted',mt)
 passed(f'{stem} unused archive members omitted')
 script=(case/'test.script').read_text(encoding='utf-8')
 vm=subprocess.run(['t32-run'],input=script,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,cwd=ROOT,check=False)
 output=vm.stdout or ''
 if log.exists(): output+='\n'+log.read_text(encoding='utf-8',errors='replace')
 if vm.returncode: fail(f'{stem} linked program executed',output)
 passed(f'{stem} linked program executed')
 for reg,val in expected.items():
  if not re.search(rf'\br{reg}\s*=\s*0x{val:08x}\b',output,re.I): fail(f'{stem} expected r{reg}=0x{val:08X}',output)
 passed(f'{stem} register results correct')
 if not re.search(r'\br15\s*=\s*0x0000f000\b',output,re.I): fail(f'{stem} stack pointer restored',output)
 passed(f'{stem} stack pointer restored')
 if not re.search(r'\bstate\s*=\s*halted\b',output,re.I): fail(f'{stem} machine halted',output)
 passed(f'{stem} machine halted')
def run_console_scroll_case():
 case=ROOT/'tests'/'03-console-scroll'; out=BUILD/'tests'; out.mkdir(parents=True,exist_ok=True)
 main_o=out/'console-scroll.o'; program=out/'console-scroll.bin'; mapf=out/'console-scroll.map'; log=ROOT/'console-scroll.log'; log.unlink(missing_ok=True)
 r=run(['t32-as','-f','obj',str(case/'main.s'),'-o',str(main_o)])
 if r.returncode: fail('console-scroll main module assembled',r.stdout)
 passed('console-scroll main module assembled')
 r=run(['t32-ld','-Ttext','0x00001000',str(main_o),str(ARCHIVE),'-Map',str(mapf),'-o',str(program)])
 if r.returncode: fail('console-scroll linked through libt32.a',r.stdout)
 passed('console-scroll linked through libt32.a')
 mt=mapf.read_text(encoding='utf-8',errors='replace')
 if 'puts' not in mt or 'putchar' not in mt: fail('console-scroll map selects console runtime',mt)
 passed('console-scroll map selects puts/putchar')
 script=(case/'test.script').read_text(encoding='utf-8')
 vm=subprocess.run(['t32-run'],input=script,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,cwd=ROOT,check=False)
 output=vm.stdout or ''
 if log.exists(): output+='\n'+log.read_text(encoding='utf-8',errors='replace')
 if vm.returncode: fail('console-scroll linked program executed',output)
 if '[ERROR]' in output: fail('console-scroll executes without framebuffer fault',output)
 passed('console-scroll executes without framebuffer fault')
 if not re.search(r'\br0\s*=\s*0x0000002a\b',output,re.I): fail('console-scroll returns expected value',output)
 passed('console-scroll returns expected value')
 if 'SCROLL LINE 01' not in output or 'SCROLL LINE 12' not in output: fail('console-scroll preserves visible output across scroll',output)
 passed('console-scroll preserves visible output across scroll')
 if not re.search(r'\bstate\s*=\s*halted\b',output,re.I): fail('console-scroll machine halted',output)
 passed('console-scroll machine halted')

def main():
 print('Running libt32 ABI 0.1 static archive validation...')
 if not ARCHIVE.is_file() or ARCHIVE.stat().st_size==0: fail('libt32.a exists')
 passed('libt32.a exists')
 listing=run(['t32-ar','t',str(ARCHIVE)]); members=[m.strip() for m in listing.stdout.splitlines() if m.strip()]
 expected_members=[
  'putchar.o','puts.o',
  'atoi.o','hex_to_string.o','string_to_hex.o',
  'memchr.o','memcmp.o','memcpy.o','memmove.o','memset.o',
  'strchr.o','strcmp.o','strcpy.o','strlen.o','strncmp.o','strncpy.o','strrev.o','strstr.o',
 ]
 if listing.returncode: fail('archive member listing succeeds',listing.stdout)
 passed('archive member listing succeeds')
 missing=[m for m in expected_members if m not in members]
 unexpected=[m for m in members if m not in expected_members]
 if missing or unexpected or len(members)!=len(expected_members):
  detail=listing.stdout
  if missing: detail+='\nmissing: '+', '.join(missing)
  if unexpected: detail+='\nunexpected: '+', '.join(unexpected)
  fail('archive contains expected ABI 0.1 members',detail)
 passed('archive contains expected ABI 0.1 members')
 run_case('00-strlen-linked',{0:5},['_start','strlen'],['strcmp','memmove'])
 run_case('01-multi-archive',{8:5,9:0},['_start','strlen','strcmp'],['memmove','strstr'])
 run_case('02-string-to-hex-abi',{0:0x1234ABCD,1:0,8:0x88888888,9:0x99999999,10:0x10101010,11:0x11111111,12:0x12121212,13:0x13131313,14:0x14141414},['_start','string_to_hex'],['strlen','strcmp'])
 run_console_scroll_case()
 print('libt32: PASS (ABI 0.1 static archive integration)'); return 0
if __name__=='__main__': sys.exit(main())
