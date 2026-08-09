from __future__ import annotations
import shutil
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(sys.argv[1]).resolve()
WORK = ROOT / "tests" / "build"

def req(cond, msg, output=""):
    if not cond:
        print(f"  FAIL {msg}")
        if output:
            print(output)
        raise SystemExit(1)
    print(f"  PASS {msg}")

def run(args=None, stdin=None):
    return subprocess.run(
        [str(EXE)] + (args or []),
        input=stdin,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        cwd=WORK,
        check=False,
    )

shutil.rmtree(WORK, ignore_errors=True)
WORK.mkdir(parents=True)

payload = WORK / "boot.bin"
payload.write_bytes(b"T32-BOOT-PAYLOAD" * 50)

print("Running t32-disk 0.0.1 validation...")

r = run(["--version"])
req(r.returncode == 0 and "0.0.1" in r.stdout, "--version reports 0.0.1", r.stdout)

script = """create disk.img 1M
format disk.img
put disk.img boot.bin BOOT.BIN
info disk.img
list disk.img
"""
r = run(stdin=script)
req(r.returncode == 0, "stdin script succeeds", r.stdout)
req((WORK/"disk.img").exists(), "disk image created")
req("T32D v0.1" in r.stdout, "info identifies T32D v0.1", r.stdout)
req("BOOT.BIN" in r.stdout, "list reports BOOT.BIN", r.stdout)

raw = (WORK/"disk.img").read_bytes()[:512]
req(raw[:4] == b"T32D", "sector 0 carries T32D fingerprint")
req(struct.unpack_from("<I", raw, 8)[0] == 512, "header records 512-byte sectors")
req(raw[32:40] == b"BOOT.BIN", "header records default boot filename")

r = run(stdin="get disk.img BOOT.BIN extracted.bin\n")
req(r.returncode == 0, "get extracts BOOT.BIN", r.stdout)
req((WORK/"extracted.bin").read_bytes() == payload.read_bytes(),
    "extracted BOOT.BIN matches original")

do_script = WORK / "do-test.script"
do_script.write_text("""create do.img 1M
format do.img
put do.img boot.bin BOOT.BIN
list do.img
""")
r = run(["do", "do-test.script"])
req(r.returncode == 0 and "BOOT.BIN" in r.stdout,
    "command-line do script succeeds", r.stdout)

nested = WORK / "nested.script"
nested.write_text("info do.img\n")
outer = WORK / "outer.script"
outer.write_text("do nested.script\n")
r = run(["do", "outer.script"])
req(r.returncode == 0 and "T32D image:" in r.stdout,
    "interactive do engine supports nested scripts", r.stdout)

bad = run(stdin="create bad.img 1M\nformat bad.img\nput bad.img missing.bin BOOT.BIN\nlist bad.img\n")
req(bad.returncode != 0, "redirected stdin fails fast on command error", bad.stdout)

print("t32-disk: PASS (13/13 cases)")
