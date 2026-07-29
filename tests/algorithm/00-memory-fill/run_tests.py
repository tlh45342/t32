import os
import re
import subprocess
import sys

VM = "t32-run"
TEST_NAME = "memory-fill"

CHECKS = [
    ("binary loaded", "loaded memory-fill.bin at 0x00001000"),
    ("machine halted", "state=halted"),
    ("HALT instruction reached", "reason=HALT instruction"),
    ("buffer base preserved", "r0 =0x00009000"),
    ("fill pointer advanced 16 bytes", "r1 =0x00009010"),
    ("fill value preserved", "r2 =0x000000a5"),
    ("fill count reached zero", "r3 =0x00000000"),
    ("verification pointer advanced 16 bytes", "r4 =0x00009010"),
    ("verification count reached zero", "r5 =0x00000000"),
    ("final comparison matched", "r6 =0x00000000"),
    ("algorithm result is PASS", "r7 =0x00000000"),
]


def run_test() -> bool:
    print("Running memory-fill validation...\n")

    for path in ("test.script", "memory-fill.bin"):
        if not os.path.exists(path):
            print(f"❌ Missing required file: {path}")
            return False

    try:
        with open("test.script", "r", encoding="utf-8") as script:
            result = subprocess.run(
                [VM],
                stdin=script,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                check=False,
            )
    except FileNotFoundError:
        print(f"❌ '{VM}' was not found in PATH.")
        return False

    if result.returncode != 0:
        print(f"❌ {VM} exited with status {result.returncode}")
        if result.stdout:
            print(result.stdout)
        return False

    print("✅ t32-run exited successfully")

    if not os.path.exists("memory-fill.log"):
        print("❌ Missing log file: memory-fill.log")
        return False

    with open("memory-fill.log", "r", encoding="utf-8") as log_file:
        log = log_file.read().lower()

    passed = True

    for label, expected in CHECKS:
        if expected.lower() in log:
            print(f"✅ {label}")
        else:
            print(f"❌ {label}")
            print(f"   Missing: {expected}")
            passed = False

    memory_match = re.search(
        r"0x00009000:\s+((?:[0-9a-f]{2}(?:\s+|$)){16})",
        log,
    )

    if memory_match:
        observed = memory_match.group(1).split()
        if observed == ["a5"] * 16:
            print("✅ host observed 16 filled bytes")
        else:
            print("❌ memory contents did not match")
            print(f"   Observed: {' '.join(observed)}")
            passed = False
    else:
        print("❌ memory dump for 0x00009000 was not found")
        passed = False

    print()
    print("PASS: memory-fill" if passed else "FAIL: memory-fill")
    return passed


if __name__ == "__main__":
    sys.exit(0 if run_test() else 1)
