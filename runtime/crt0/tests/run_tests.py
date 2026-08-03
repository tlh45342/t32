from __future__ import annotations

import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "build"
CRT0 = BUILD / "crt0.o"
MAIN_SOURCE = ROOT / "tests" / "main.s"
MAIN_OBJECT = BUILD / "tests" / "main.o"
PROGRAM = BUILD / "tests" / "crt0-main.bin"
MAP = BUILD / "tests" / "crt0-main.map"


def passed(message: str) -> None:
    print(f"  PASS {message}")


def failed(message: str, output: str = "") -> None:
    print(f"  FAIL {message}")
    if output:
        print("\n--- output ---")
        print(output.rstrip())
        print("--- end output ---")
    raise SystemExit(1)


def require(condition: bool, message: str, output: str = "") -> None:
    if not condition:
        failed(message, output)
    passed(message)


def tool(name: str) -> str:
    found = shutil.which(name)
    if not found:
        failed(f"required tool not found in PATH: {name}")
    return found


def run(args: list[str], *, input_text: str | None = None) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        args,
        input=input_text,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        cwd=ROOT,
        check=False,
    )


def register_value(output: str, register: int) -> int | None:
    match = re.search(rf"\br{register}\s*=\s*(0x[0-9a-fA-F]+|\d+)\b", output)
    return int(match.group(1), 0) if match else None


def link_program() -> subprocess.CompletedProcess[str]:
    linker = tool("t32-ld")
    candidates = [
        [linker, "-o", str(PROGRAM), "-Map", str(MAP), str(CRT0), str(MAIN_OBJECT)],
        [linker, str(CRT0), str(MAIN_OBJECT), "-o", str(PROGRAM), "-Map", str(MAP)],
    ]

    last: subprocess.CompletedProcess[str] | None = None
    for args in candidates:
        PROGRAM.unlink(missing_ok=True)
        MAP.unlink(missing_ok=True)
        last = run(args)
        if last.returncode == 0 and PROGRAM.exists():
            return last

    assert last is not None
    return last


def run_linked_program() -> subprocess.CompletedProcess[str]:
    monitor_script = "\n".join([
        "reset",
        f"load {PROGRAM} 0x00001000",
        "set pc 0x00001000",
        "run",
        "regs",
        "status",
        "quit",
        "",
    ])
    return run([tool("t32-run")], input_text=monitor_script)


def main() -> int:
    print("Running crt0 ABI 0.1 integration validation...")

    require(CRT0.exists(), "crt0.o exists")
    MAIN_OBJECT.parent.mkdir(parents=True, exist_ok=True)

    assembled = run([
        tool("t32-as"), "-f", "obj", str(MAIN_SOURCE), "-o", str(MAIN_OBJECT)
    ])
    require(
        assembled.returncode == 0 and MAIN_OBJECT.exists(),
        "test main module assembled",
        assembled.stdout,
    )

    inspected = run([tool("t32-nm"), str(CRT0)])
    require(inspected.returncode == 0, "crt0.o inspected", inspected.stdout)
    require(re.search(r"\b_start\b", inspected.stdout) is not None,
            "_start global symbol visible", inspected.stdout)
    require(re.search(r"\bmain\b", inspected.stdout) is not None,
            "main external reference visible", inspected.stdout)

    linked = link_program()
    require(linked.returncode == 0 and PROGRAM.exists(),
            "crt0.o and main.o linked", linked.stdout)
    require(MAP.exists(), "link map emitted")

    map_text = MAP.read_text(encoding="utf-8", errors="replace")
    require("_start" in map_text, "_start appears in map", map_text)
    require("main" in map_text, "main appears in map", map_text)

    executed = run_linked_program()
    require(executed.returncode == 0, "linked program executed", executed.stdout)
    require(re.search(r"\bstate\s*=\s*halted\b", executed.stdout, re.IGNORECASE) is not None,
            "machine halted", executed.stdout)
    require(re.search(r"\breason\s*=\s*HALT instruction\b", executed.stdout, re.IGNORECASE) is not None,
            "HALT instruction reached", executed.stdout)

    r0 = register_value(executed.stdout, 0)
    require(r0 == 5, "main return value remains in r0", executed.stdout)

    r15 = register_value(executed.stdout, 15)
    require(r15 == 0x0000F000, "stack pointer restored", executed.stdout)

    print("crt0: PASS (13/13 cases)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
