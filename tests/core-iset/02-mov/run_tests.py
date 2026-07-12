import os
import subprocess
import sys

VM = "t32-run"
TEST_NAME = "mov"

CHECKS = [
    ("binary loaded", "loaded mov.bin at 0x00001000"),
    ("r0 unchanged", "r0 =0x00000000"),
    ("r1 copied", "r1 =0x00000000"),
    ("program counter", "pc =0x00001008"),
    ("halted state", "state=halted"),
    ("instruction count", "instructions=2"),
    ("carry", "carry=0"),
    ("zero", "zero=0"),
    ("negative", "negative=0"),
    ("overflow", "overflow=0"),
]

def run_test():
    log_path = f"{TEST_NAME}.log"

    subprocess.run(
        [VM],
        stdin=open("test.script", "r"),
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=False,
    )

    if not os.path.exists(log_path):
        print(f"Missing log file: {log_path}")
        return False

    log = open(log_path, "r", encoding="utf-8").read()

    passed = True
    for label, expected in CHECKS:
        if expected not in log:
            print(f"FAIL {label}: missing {expected}")
            passed = False
        else:
            print(f"PASS {label}")

    return passed

if __name__ == "__main__":
    sys.exit(0 if run_test() else 1)