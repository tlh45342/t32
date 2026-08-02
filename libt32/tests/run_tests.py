from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "build"
ARCHIVE = BUILD / "libt32.a"


def passed(message: str) -> None:
    print(f"  PASS {message}")


def fail(message: str, output: str = "") -> None:
    print(f"  FAIL {message}")
    if output:
        print("\n--- output ---")
        print(output.rstrip())
        print("--- end output ---")
    raise SystemExit(1)


def run(command: list[str]) -> subprocess.CompletedProcess[str]:
    return subprocess.run(command, cwd=ROOT, text=True, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, check=False)


def run_case(name: str, expected: dict[int, int], required_symbols: list[str],
             forbidden_symbols: list[str]) -> None:
    case = ROOT / "tests" / name
    stem = name.split("-", 1)[-1]
    main_o = BUILD / "tests" / f"{stem}.o"
    program = BUILD / "tests" / f"{stem}.bin"
    map_file = BUILD / "tests" / f"{stem}.map"
    log = ROOT / f"{stem}.log"
    BUILD.joinpath("tests").mkdir(parents=True, exist_ok=True)
    log.unlink(missing_ok=True)

    result = run(["t32-as", "-f", "obj", str(case / "main.s"), "-o", str(main_o)])
    if result.returncode != 0:
        fail(f"{stem} main module assembled", result.stdout)
    passed(f"{stem} main module assembled")

    result = run(["t32-ld", "-Ttext", "0x00001000", str(main_o), str(ARCHIVE),
                  "-Map", str(map_file), "-o", str(program)])
    if result.returncode != 0:
        fail(f"{stem} linked through libt32.a", result.stdout)
    passed(f"{stem} linked through libt32.a")

    map_text = map_file.read_text(encoding="utf-8", errors="replace")
    for symbol in required_symbols:
        if symbol not in map_text:
            fail(f"{stem} map contains {symbol}", map_text)
    passed(f"{stem} required archive members selected")
    for symbol in forbidden_symbols:
        if re.search(rf"\b{re.escape(symbol)}\b", map_text):
            fail(f"{stem} unused symbol {symbol} was extracted", map_text)
    passed(f"{stem} unused archive members omitted")

    script = (case / "test.script").read_text(encoding="utf-8")
    vm = subprocess.run(["t32-run"], input=script, text=True,
                        stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                        cwd=ROOT, check=False)
    output = vm.stdout or ""
    if log.exists():
        output += "\n" + log.read_text(encoding="utf-8", errors="replace")
    if vm.returncode != 0:
        fail(f"{stem} linked program executed", output)
    passed(f"{stem} linked program executed")

    for reg, value in expected.items():
        if not re.search(rf"\br{reg}\s*=\s*0x{value:08x}\b", output, re.I):
            fail(f"{stem} expected r{reg}=0x{value:08X}", output)
    passed(f"{stem} register results correct")

    if not re.search(r"\br15\s*=\s*0x0000f000\b", output, re.I):
        fail(f"{stem} stack pointer restored", output)
    passed(f"{stem} stack pointer restored")
    if not re.search(r"\bstate\s*=\s*halted\b", output, re.I):
        fail(f"{stem} machine halted", output)
    passed(f"{stem} machine halted")


def main() -> int:
    print("Running libt32 static archive validation...")
    if not ARCHIVE.is_file() or ARCHIVE.stat().st_size == 0:
        fail("libt32.a exists")
    passed("libt32.a exists")

    listing = run(["t32-ar", "t", str(ARCHIVE)])
    members = listing.stdout.splitlines()
    if listing.returncode != 0 or len(members) != 16 or "strlen.o" not in members:
        fail("archive contains sixteen members", listing.stdout)
    passed("archive contains sixteen members")

    run_case("00-strlen-linked", {1: 5}, ["_start", "strlen"], ["strcmp", "memmove"])
    run_case("01-multi-archive", {8: 5, 9: 0}, ["_start", "strlen", "strcmp"], ["memmove", "strstr"])

    print("libt32: PASS (static archive integration)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
