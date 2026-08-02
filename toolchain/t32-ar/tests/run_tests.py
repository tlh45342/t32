from __future__ import annotations
import shutil
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EXE = ROOT / "t32-ar.exe"

def run(args, cwd=None):
    return subprocess.run(
        [str(EXE), *args],
        cwd=cwd or ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )

def put16(buf, off, value):
    struct.pack_into("<H", buf, off, value)

def put32(buf, off, value):
    struct.pack_into("<I", buf, off, value)

def make_obj(path: Path, global_name: str):
    strings = b"\0.text\0.data\0.bss\0" + global_name.encode() + b"\0"
    text_name = 1
    data_name = 7
    bss_name = 13
    sym_name = 18
    header = bytearray(48)
    header[:8] = b"T32OBJ\0\0"
    put16(header, 8, 1)
    put16(header, 10, 0)
    put32(header, 12, 1)
    put32(header, 16, 4)
    put32(header, 20, 2)
    put32(header, 24, 0)
    put32(header, 28, 48)
    put32(header, 32, 48 + 4*32)
    put32(header, 36, 48 + 4*32 + 2*24)
    put32(header, 40, 48 + 4*32 + 2*24)
    put32(header, 44, len(strings))

    sections = bytearray(4*32)
    # null section stays zero
    for idx, (name, stype, flags, align) in enumerate([
        (text_name, 1, 3, 4),
        (data_name, 1, 5, 1),
        (bss_name, 2, 5, 4),
    ], start=1):
        off = idx*32
        put32(sections, off+0, name)
        put32(sections, off+4, stype)
        put32(sections, off+8, flags)
        put32(sections, off+12, align)

    symbols = bytearray(2*24)
    put32(symbols, 24+0, sym_name)
    put32(symbols, 24+4, 1)
    put32(symbols, 24+8, 0)
    symbols[24+16] = 1
    symbols[24+17] = 1

    path.write_bytes(header + sections + symbols + strings)

def check(cond, name):
    if not cond:
        print(f"  FAIL {name}")
        raise SystemExit(1)
    print(f"  PASS {name}")

def main():
    print("Running t32-ar validation...")
    with tempfile.TemporaryDirectory() as td:
        d = Path(td)
        a = d / "a.o"
        b = d / "b.o"
        make_obj(a, "alpha")
        make_obj(b, "beta")
        ar = d / "libtest.a"

        r = run(["--version"])
        check(r.returncode == 0 and "t32-ar 0.0.1" in r.stdout, "version")

        r = run(["rcs", str(ar), str(a), str(b)])
        check(r.returncode == 0 and ar.exists(), "archive create")

        r = run(["t", str(ar)])
        check(r.returncode == 0 and r.stdout.splitlines() == ["a.o", "b.o"], "member list")

        raw1 = ar.read_bytes()
        r = run(["rcs", str(ar), str(a), str(b)])
        raw2 = ar.read_bytes()
        check(r.returncode == 0 and raw1 == raw2, "deterministic rebuild")

        extract_dir = d / "extract"
        extract_dir.mkdir()
        r = run(["x", str(ar)], cwd=extract_dir)
        check(r.returncode == 0 and (extract_dir/"a.o").read_bytes() == a.read_bytes(), "extract all")

        one_dir = d / "one"
        one_dir.mkdir()
        r = run(["x", str(ar), "b.o"], cwd=one_dir)
        check(r.returncode == 0 and (one_dir/"b.o").exists() and not (one_dir/"a.o").exists(), "extract selected")

        r = run(["d", str(ar), "a.o"])
        check(r.returncode == 0, "delete member")
        r = run(["t", str(ar)])
        check(r.stdout.splitlines() == ["b.o"], "list after delete")

        bad = d / "bad.o"
        bad.write_bytes(b"not-an-object")
        r = run(["rcs", str(d/"bad.a"), str(bad)])
        check(r.returncode != 0, "reject non-object member")

        corrupt = d / "corrupt.a"
        corrupt.write_bytes(b"bad")
        r = run(["t", str(corrupt)])
        check(r.returncode != 0, "reject corrupt archive")

        r = run(["x", str(ar), "missing.o"], cwd=d)
        check(r.returncode != 0, "missing member rejected")

    print("t32-ar: PASS (10/10 cases)")
    return 0

if __name__ == "__main__":
    sys.exit(main())
