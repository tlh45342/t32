import os
import subprocess
import sys

VM = "t32-run"
TEST_NAME = "r0_42"

CHECKS = [
    ("MOVI", "MOVI r0"),
    ("HALT", "HALT"),
    ("R0", "r0=0x0000002A"),
    ("PC", "pc=0x0000100C"),
    ("STATE", "state=halted"),
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