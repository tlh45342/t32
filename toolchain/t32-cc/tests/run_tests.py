#!/usr/bin/env python3
"""Regression tests for the Stage 1 t32-cc compiler."""

from __future__ import annotations

import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent
CASES = ROOT / "cases"
EXPECTED = ROOT / "expected"
BUILD = ROOT / "build"


def run(command: list[str]) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        command,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )


def require(condition: bool, name: str, detail: str = "") -> None:
    if condition:
        print(f"  PASS {name}")
        return
    print(f"  FAIL {name}")
    if detail:
        print(detail.rstrip())
    raise SystemExit(1)


def compile_case(compiler: Path, source: str, output: str, *extra: str):
    destination = BUILD / output
    result = run([
        str(compiler),
        *extra,
        str(CASES / source),
        "-o",
        str(destination),
    ])
    return result, destination


def main() -> int:
    compiler = Path(sys.argv[1] if len(sys.argv) > 1 else "./t32-cc").resolve()
    require(compiler.is_file(), "compiler exists", str(compiler))

    if BUILD.exists():
        shutil.rmtree(BUILD)
    BUILD.mkdir(parents=True)

    print("Running t32-cc Stage 1 tests...")

    version = run([str(compiler), "--version"])
    require(version.returncode == 0, "--version exits successfully", version.stderr)
    require("t32-cc 0.0.3" in version.stdout, "--version reports 0.0.3", version.stdout)

    result, output = compile_case(compiler, "return_42.c", "return_42.s")
    require(result.returncode == 0, "compile decimal return", result.stderr)
    require(output.read_text() == (EXPECTED / "return_42.s").read_text(),
            "decimal assembly matches", output.read_text())

    result, output = compile_case(compiler, "return_0.c", "return_0.s")
    require(result.returncode == 0, "compile zero return", result.stderr)
    require("movi r0, 0x00000000" in output.read_text(), "zero encoded")

    result, output = compile_case(compiler, "return_hex.c", "return_hex.s")
    require(result.returncode == 0, "compile comments and hex", result.stderr)
    require("movi r0, 0x00001234" in output.read_text(), "hex encoded")

    result, output = compile_case(compiler, "return_negative.c", "return_negative.s")
    require(result.returncode == 0, "compile negative return", result.stderr)
    require("movi r0, 0xFFFFFFFF" in output.read_text(), "negative encoded")

    result, output = compile_case(
        compiler, "return_42.c", "return_42_origin.s", "--origin", "0x1000"
    )
    require(result.returncode == 0, "compile custom origin", result.stderr)
    require(".org 0x00001000" in output.read_text(), "custom origin emitted")

    result, _ = compile_case(compiler, "invalid_expression.c", "bad_expression.s")
    require(result.returncode != 0, "reject unsupported expression")
    require("integer literal" in result.stderr, "expression diagnostic", result.stderr)

    result, _ = compile_case(compiler, "invalid_function.c", "bad_function.s")
    require(result.returncode != 0, "reject non-main function")
    require("expected 'main'" in result.stderr, "function diagnostic", result.stderr)

    result, _ = compile_case(compiler, "invalid_missing_semicolon.c", "bad_semicolon.s")
    require(result.returncode != 0, "reject missing semicolon")
    require("expected ';'" in result.stderr, "semicolon diagnostic", result.stderr)

    print("All t32-cc Stage 1 tests passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
