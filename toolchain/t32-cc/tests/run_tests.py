#!/usr/bin/env python3
from __future__ import annotations

import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
CASES = ROOT / "cases"
EXPECTED = ROOT / "expected"
BUILD = ROOT / "build"
PASS_COUNT = 0


def run(cmd, env=None, input_text=None):
    return subprocess.run(
        cmd,
        input=input_text,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
        env=env,
    )


def req(condition, name, detail=""):
    global PASS_COUNT
    if condition:
        PASS_COUNT += 1
        print(f"  PASS {name}")
        return
    print(f"  FAIL {name}")
    if detail:
        print(detail.rstrip())
    raise SystemExit(1)


def reg(text, number):
    match = re.search(rf"\br{number}\s*=\s*(0x[0-9a-fA-F]+|\d+)\b", text)
    return int(match.group(1), 0) if match else None


def execute(binary):
    script = "\n".join(
        [
            "reset",
            f"load {binary} 0x00001000",
            "set pc 0x00001000",
            "run",
            "regs",
            "status",
            "quit",
            "",
        ]
    )
    result = run(["t32-run"], input_text=script)
    return result, result.stdout + result.stderr


def compile_and_execute(cc, case_name, expected_r0, label):
    binary = BUILD / f"{case_name}.bin"
    result = run([str(cc), str(CASES / f"{case_name}.c"), "-o", str(binary)])
    req(result.returncode == 0 and binary.exists(), f"{label} program links", result.stderr)
    execution, output = execute(binary)
    req(execution.returncode == 0, f"{label} program executes", output)
    req(reg(output, 0) == expected_r0, f"{label} returns expected value", output)
    req(reg(output, 15) == 0xF000, f"{label} restores stack", output)
    req(re.search(r"state\s*=\s*halted", output, re.I) is not None, f"{label} halts", output)


def compare_assembly(cc, case_name, label):
    assembly = BUILD / f"{case_name}.s"
    result = run([str(cc), "-S", str(CASES / f"{case_name}.c"), "-o", str(assembly)])
    req(result.returncode == 0, f"{label} -S compile succeeds", result.stderr)
    req(result.stdout == "", f"{label} remains quiet by default", result.stdout)
    expected = EXPECTED / f"{case_name}.s"
    req(assembly.read_text() == expected.read_text(), f"{label} assembly matches", assembly.read_text())
    return assembly.read_text()


def main():
    cc = Path(sys.argv[1] if len(sys.argv) > 1 else "./t32-cc").resolve()
    req(cc.is_file(), "compiler exists", str(cc))

    shutil.rmtree(BUILD, ignore_errors=True)
    BUILD.mkdir(parents=True)

    print("Running t32-cc Stage 5 addition-expression tests...")

    version = run([str(cc), "--version"])
    req(version.returncode == 0, "--version exits successfully", version.stderr)
    req("t32-cc 0.4.0" in version.stdout, "--version reports 0.4.0", version.stdout)

    # Preserve earlier compiler milestones exactly.
    compare_assembly(cc, "return_42", "constant return")
    local_text = compare_assembly(cc, "local_x", "local variable")
    req("subi r15, r15, 4" in local_text, "local allocates four-byte stack slot")
    req("stw  r1, [r15]" in local_text, "initializer stored to local slot")
    req("ldw  r0, [r15]" in local_text, "local loaded into return register")
    req("addi r15, r15, 4" in local_text, "local stack slot released")

    assignment_text = compare_assembly(cc, "assignment", "literal assignment")
    req(assignment_text.count("stw  r1, [r15]") == 2, "literal assignment emits second store")

    compile_and_execute(cc, "local_x", 5, "local")
    compile_and_execute(cc, "local_negative", 0xFFFFFFF9, "negative local")
    compile_and_execute(cc, "assignment", 7, "literal assignment")
    compile_and_execute(cc, "assignment_repeated", 3, "repeated literal assignment")
    compile_and_execute(cc, "assignment_negative", 0xFFFFFFF7, "negative assignment")

    # First binary expression: one '+' and two simple operands.
    constant_add = compare_assembly(cc, "return_add_constants", "constant plus constant")
    req("movi r0, 0x00000005" in constant_add, "constant addition loads left operand")
    req("movi r1, 0x00000003" in constant_add, "constant addition loads right operand")
    req("add  r0, r0, r1" in constant_add, "constant addition emits ADD")

    local_add = compare_assembly(cc, "local_add_constant", "local plus constant")
    req("ldw  r0, [r15]" in local_add, "local addition loads local operand")
    req("add  r0, r0, r1" in local_add, "local addition emits ADD")

    constant_local = compare_assembly(cc, "constant_add_local", "constant plus local")
    req("movi r0, 0x00000003" in constant_local, "constant-left addition loads literal")
    req("ldw  r1, [r15]" in constant_local, "constant-left addition loads local")

    local_local = compare_assembly(cc, "local_add_local", "local plus local")
    req(local_local.count("ldw  r") == 2, "local plus local loads both operands")

    assignment_add = compare_assembly(cc, "assignment_add", "addition assignment")
    req("ldw  r1, [r15]" in assignment_add, "addition assignment loads current value")
    req("movi r2, 0x00000001" in assignment_add, "addition assignment loads increment")
    req("add  r1, r1, r2" in assignment_add, "addition assignment emits ADD")
    req(assignment_add.count("stw  r1, [r15]") == 2, "addition assignment stores updated value")

    # Object mode still exports main for expression code.
    obj = BUILD / "local_add_constant.o"
    result = run([str(cc), "-c", str(CASES / "local_add_constant.c"), "-o", str(obj)])
    req(result.returncode == 0 and obj.exists(), "-c emits expression object", result.stderr)
    symbols = run(["t32-nm", str(obj)])
    req(symbols.returncode == 0 and re.search(r"\bmain\b", symbols.stdout), "expression object exports main", symbols.stdout + symbols.stderr)

    # Execute all supported operand combinations and assignment forms.
    compile_and_execute(cc, "return_add_constants", 8, "constant plus constant")
    compile_and_execute(cc, "local_add_constant", 8, "local plus constant")
    compile_and_execute(cc, "constant_add_local", 8, "constant plus local")
    compile_and_execute(cc, "local_add_local", 10, "local plus local")
    compile_and_execute(cc, "assignment_add", 6, "addition assignment")
    compile_and_execute(cc, "assignment_identifier_rhs", 5, "identifier assignment")
    compile_and_execute(cc, "assignment_expression_rhs", 9, "constant-expression assignment")
    compile_and_execute(cc, "assignment_repeated_increment", 3, "repeated increment")
    compile_and_execute(cc, "add_negative", 0xFFFFFFFB, "negative addition")

    # Verbose mode remains opt-in.
    verbose_obj = BUILD / "verbose.o"
    verbose = run([str(cc), "-v", "-c", str(CASES / "local_add_constant.c"), "-o", str(verbose_obj)])
    req(verbose.returncode == 0 and "invoke: t32-as" in verbose.stdout, "-v shows invoked phase", verbose.stdout + verbose.stderr)

    # Semantic and syntax negatives.
    negatives = [
        ("assignment_undeclared.c", "assignment to undeclared local variable", "undeclared assignment"),
        ("expression_undeclared_rhs.c", "use of undeclared local variable", "undeclared expression operand"),
        ("expression_chained.c", "one binary '+' operator", "chained addition"),
        ("expression_parenthesized.c", "expected integer literal or local variable", "parenthesized expression"),
        ("expression_subtraction.c", "expected ';'", "subtraction expression"),
        ("expression_multiply.c", "invalid token", "multiplication expression"),
        ("undeclared_local.c", "undeclared local variable", "undeclared local"),
        ("two_locals.c", "exactly one local variable", "second local"),
        ("local_missing_initializer.c", "expected '='", "missing initializer"),
        ("invalid_function.c", "expected 'main'", "non-main function"),
        ("invalid_missing_semicolon.c", "expected ';'", "missing semicolon"),
    ]

    for case_name, needle, label in negatives:
        destination = BUILD / f"negative_{case_name}.s"
        result = run([str(cc), "-S", str(CASES / case_name), "-o", str(destination)])
        req(result.returncode != 0, f"reject {label}")
        req(needle in result.stderr, f"{label} diagnostic", result.stderr)
        req(not destination.exists(), f"{label} leaves no output")

    mode_conflict = run([str(cc), "-S", "-c", str(CASES / "return_0.c")])
    req(mode_conflict.returncode != 0, "reject -S with -c")
    req("cannot be used together" in mode_conflict.stderr, "mode conflict diagnostic", mode_conflict.stderr)

    missing_binary = BUILD / "missing.bin"
    environment = os.environ.copy()
    environment["T32_PREFIX"] = str(BUILD / "no-runtime")
    missing_runtime = run(
        [str(cc), str(CASES / "return_0.c"), "-o", str(missing_binary)],
        env=environment,
    )
    req(missing_runtime.returncode != 0, "missing runtime rejected")
    req("missing startup object" in missing_runtime.stderr, "missing runtime diagnostic", missing_runtime.stderr)
    req(not missing_binary.exists(), "failed link leaves no binary")

    absent = run([str(cc), "-S", str(CASES / "does-not-exist.c")])
    req(absent.returncode != 0, "missing source rejected")
    req("cannot open" in absent.stderr, "missing source diagnostic", absent.stderr)

    print(f"t32-cc: PASS ({PASS_COUNT}/{PASS_COUNT} cases)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
