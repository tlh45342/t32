import os
import subprocess
import sys
from pathlib import Path

ASM = os.environ.get("T32_AS", "t32-as")
VM = os.environ.get("T32_RUN", "t32-run")
LOAD_ADDRESS = 0x00001000

CASES = (
    ("opcode_05", "reserved_05.bin", 0x05),
    ("opcode_06", "reserved_06.bin", 0x06),
    ("opcode_07", "reserved_07.bin", 0x07),
)

def initialize():
    for stream in (sys.stdin, sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8")
        except Exception:
            pass

def run_process(args, *, input_text=None):
    return subprocess.run(
        args,
        input=input_text,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=False,
    )


def check(label, condition, detail=""):
    if condition:
        print(f"    PASS {label}")
        return True

    suffix = f": {detail}" if detail else ""
    print(f"    FAIL {label}{suffix}")
    return False


def expected_word(opcode):
    return bytes((0x00, 0x00, 0x00, opcode))


def run_vm(binary_name, log_name):
    Path(log_name).unlink(missing_ok=True)

    script = "\n".join(
        (
            "reset",
            f"logfile {log_name}",
            f"load {binary_name} 0x{LOAD_ADDRESS:08x}",
            f"set pc 0x{LOAD_ADDRESS:08x}",
            "run 2",
            "regs",
            "quit",
            "",
        )
    )

    result = run_process([VM], input_text=script)

    log_text = ""
    log_path = Path(log_name)
    if log_path.exists():
        log_text = log_path.read_text(encoding="utf-8", errors="replace")

    return result, result.stdout + "\n" + log_text


def validate_reserved_opcode(name, binary_name, opcode):
    print(f"  {name}")
    passed = True

    data = Path(binary_name).read_bytes()
    passed &= check(
        "assembler raw-word encoding",
        len(data) >= 4 and data[:4] == expected_word(opcode),
        f"got {data[:4].hex(' ')}",
    )

    log_name = f"{name}.log"
    result, combined = run_vm(binary_name, log_name)
    output = combined.lower()

    passed &= check(
        "binary loaded",
        f"loaded {binary_name.lower()} at 0x{LOAD_ADDRESS:08x}" in output,
    )
    passed &= check(
        "pc set",
        f"pc <= 0x{LOAD_ADDRESS:08x}" in output,
    )
    passed &= check(
        "run rejected reserved opcode",
        f"[error] run failed: unknown opcode 0x{opcode:02x} "
        f"at 0x{LOAD_ADDRESS:08x}" in output,
    )
    passed &= check(
        "pc advanced one word",
        f"pc =0x{LOAD_ADDRESS + 4:08x}" in output
        or f"pc=0x{LOAD_ADDRESS + 4:08x}" in output,
    )
    passed &= check(
        "following halt not executed",
        "state=halted" not in output,
    )

    if not passed:
        print("    --- t32-run output ---")
        for line in combined.splitlines():
            print(f"    {line}")

    Path(log_name).unlink(missing_ok=True)
    return bool(passed)


def validate_unknown_mnemonics():
    print("  assembler_rejects_reserved_mnemonics")
    passed = True

    for opcode in (0x05, 0x06, 0x07):
        source = Path(f"__reserved_mnemonic_{opcode:02x}.s")
        binary = Path(f"__reserved_mnemonic_{opcode:02x}.bin")

        source.write_text(f"op{opcode:02x}\n", encoding="utf-8")
        binary.unlink(missing_ok=True)

        try:
            result = run_process(
                [ASM, "-f", "bin", str(source), "-o", str(binary)]
            )
            output = result.stdout.lower()

            case_ok = (
                result.returncode != 0
                and "unknown instruction" in output
            )

            passed &= check(
                f"op{opcode:02x} rejected",
                case_ok,
                f"returncode={result.returncode}",
            )

            if not case_ok:
                for line in result.stdout.splitlines():
                    print(f"      {line}")
        finally:
            source.unlink(missing_ok=True)
            binary.unlink(missing_ok=True)

    return bool(passed)


def main():
    initialize()
    print("Running reserved system opcode validation...")

    results = [
        validate_reserved_opcode(name, binary, opcode)
        for name, binary, opcode in CASES
    ]
    results.append(validate_unknown_mnemonics())

    passed_count = sum(1 for result in results if result)
    total = len(results)

    if all(results):
        print(f"reserved-system: PASS ({passed_count}/{total} cases)")
        return 0

    print(f"reserved-system: FAIL ({passed_count}/{total} cases)")
    return 1


if __name__ == "__main__":
    sys.exit(main())
