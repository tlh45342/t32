import subprocess
import os
import sys

VM = "t32-run.exe"

TEST_NAME = "halt"

CHECKS = [
    ("binary loaded", "loaded halt.bin at 0x00001000"),
    ("program counter", "pc =0x00001004"),
    ("halted state", "state=halted"),
    ("instruction count", "instructions=1"),
    ("carry unchanged", "carry=0"),
    ("zero unchanged", "zero=0"),
    ("negative unchanged", "negative=0"),
    ("overflow unchanged", "overflow=0"),
    ("halt reason", "reason=HALT instruction"),
]

def run_test():
    print(f"Running {TEST_NAME}...")
    script_path = f"test.script"
    bin_path = f"{TEST_NAME}.bin"
    log_path = f"{TEST_NAME}.log"

    if not os.path.exists(script_path):
        print(f"❌ Missing script: {script_path}")
        return False

    if not os.path.exists(bin_path):
        print(f"❌ Missing binary: {bin_path}")
        return False

    try:
        subprocess.run(
            [VM],
            stdin=open(script_path, "r"),
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True
        )
    except FileNotFoundError:
        print(f"❌ Error: '{VM}' not found in PATH.")
        return False

    if not os.path.exists(log_path):
        print(f"❌ Missing log file: {log_path}")
        return False

    with open(log_path, "r") as f:
        log = f.read()

    passed = True
    for label, expected in CHECKS:
        if expected not in log:
            print(f"  ❌ Check failed: {label}")
            print(f"     Missing: {expected}")
            passed = False
        else:
            print(f"  ✅ {label}")

    print(f"{TEST_NAME}: {'✅ passed' if passed else '❌ failed'}\n")
    return passed

if __name__ == "__main__":
    success = run_test()
    sys.exit(0 if success else 1)