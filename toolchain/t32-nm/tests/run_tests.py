from __future__ import annotations

import os
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
EXE = ROOT / ("t32-nm.exe" if os.name == "nt" else "t32-nm")


def u32(v: int) -> bytes:
    return struct.pack("<I", v & 0xFFFFFFFF)


def make_object(path: Path) -> None:
    strings = b"\0.text\0.data\0.bss\0_start\0message\0strlen\0"
    offsets = {}
    for name in [b".text", b".data", b".bss", b"_start", b"message", b"strlen"]:
        offsets[name.decode()] = strings.index(name)

    section_count, symbol_count, reloc_count = 4, 4, 2
    sec_off = 48
    sym_off = sec_off + section_count * 32
    rel_off = sym_off + symbol_count * 24
    str_off = rel_off + reloc_count * 20
    data_off = (str_off + len(strings) + 3) & ~3
    text = b"\x09\x00\x00\x00" + b"\0" * 4 + b"\x32\0\0\0" + b"\0" * 4
    data = b"Hello\0"
    data_data_off = (data_off + len(text) + 3) & ~3

    out = bytearray()
    out += b"T32OBJ\0\0"
    out += struct.pack("<HH", 1, 0)
    out += struct.pack("<9I", 1, section_count, symbol_count, reloc_count,
                       sec_off, sym_off, rel_off, str_off, len(strings))

    def sec(name, typ, flags, align, size, off):
        return struct.pack("<8I", name, typ, flags, align, size, off, 0, 0)

    out += sec(0, 0, 0, 0, 0, 0)
    out += sec(offsets[".text"], 1, 3, 4, len(text), data_off)
    out += sec(offsets[".data"], 1, 5, 1, len(data), data_data_off)
    out += sec(offsets[".bss"], 2, 5, 4, 16, 0)

    def sym(name, section, value, size, binding=0, typ=0):
        return struct.pack("<4IBBH I", name, section, value, size, binding, typ, 0, 0)

    out += sym(0, 0, 0, 0)
    out += sym(offsets["_start"], 1, 0, 0, 1, 1)
    out += sym(offsets["message"], 2, 0, 6, 0, 2)
    out += sym(offsets["strlen"], 0, 0, 0, 1, 1)

    out += struct.pack("<4Ii", 1, 4, 2, 3, 0)
    out += struct.pack("<4Ii", 1, 12, 3, 2, 0)
    out += strings
    while len(out) % 4:
        out.append(0)
    assert len(out) == data_off
    out += text
    while len(out) % 4:
        out.append(0)
    assert len(out) == data_data_off
    out += data
    path.write_bytes(out)


def run(*args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run([str(EXE), *args], text=True,
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                          cwd=ROOT, check=False)


def check(label: str, condition: bool, output: str = "") -> None:
    if not condition:
        print(f"  FAIL {label}")
        if output:
            print(output)
        raise SystemExit(1)
    print(f"  PASS {label}")


def main() -> int:
    print("Running t32-nm validation...")
    with tempfile.TemporaryDirectory() as td:
        td = Path(td)
        obj = td / "sample.o"
        make_object(obj)

        r = run("--version")
        check("version", r.returncode == 0 and "t32-nm 0.0.1" in r.stdout, r.stdout)

        r = run(str(obj))
        check("default display", r.returncode == 0 and "Sections" in r.stdout and
              "Symbols" in r.stdout and "Relocations" in r.stdout, r.stdout)
        check("section decoding", ".text" in r.stdout and ".data" in r.stdout and
              ".bss" in r.stdout and "NOBITS" in r.stdout, r.stdout)
        check("symbol decoding", "T _start" in r.stdout and "d message" in r.stdout and
              "U strlen" in r.stdout, r.stdout)
        check("relocation decoding", "R_T32_ADDR32" in r.stdout and
              "R_T32_TARGET32" in r.stdout, r.stdout)

        r = run("--symbols", str(obj))
        check("symbols-only view", r.returncode == 0 and "Symbols" in r.stdout and
              "Sections" not in r.stdout and "Relocations" not in r.stdout, r.stdout)

        bad = td / "bad-magic.o"
        b = bytearray(obj.read_bytes()); b[0] = ord("X"); bad.write_bytes(b)
        r = run(str(bad))
        check("invalid magic rejected", r.returncode != 0 and "invalid T32OBJ magic" in r.stdout, r.stdout)

        bad = td / "bad-version.o"
        b = bytearray(obj.read_bytes()); b[8:10] = struct.pack("<H", 2); bad.write_bytes(b)
        r = run(str(bad))
        check("unsupported version rejected", r.returncode != 0 and "unsupported T32OBJ major version" in r.stdout, r.stdout)

        bad = td / "truncated.o"
        bad.write_bytes(obj.read_bytes()[:30])
        r = run(str(bad))
        check("truncated header rejected", r.returncode != 0 and "smaller than T32OBJ header" in r.stdout, r.stdout)

        bad = td / "bad-reloc-symbol.o"
        b = bytearray(obj.read_bytes())
        rel_off = struct.unpack_from("<I", b, 36)[0]
        struct.pack_into("<I", b, rel_off + 8, 99)
        bad.write_bytes(b)
        r = run(str(bad))
        check("bad relocation symbol rejected", r.returncode != 0 and "relocation symbol index is invalid" in r.stdout, r.stdout)

    print("t32-nm: PASS (10/10 cases)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
