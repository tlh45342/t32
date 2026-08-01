import os
import subprocess
import sys

VM = "t32-run"
LOG_FILE = "string-compare.log"

CHECKS = [
    ("binary loaded", "loaded string-compare.bin at 0x00001000"),
    ("machine halted", "state=halted"),
    ("HALT instruction reached", "reason=HALT instruction"),
    ("empty strings compare equal", "r8 =0x00000000"),
    ("one-character strings compare equal", "r9 =0x00000000"),
    ("identical words compare equal", "r10=0x00000000"),
    ("different character is detected", "r11=0x00000001"),
    ("left-shorter string is detected", "r12=0x00000001"),
    ("right-shorter string is detected", "r13=0x00000001"),
    ("all six test cases completed", "r14=0x00000006"),
    ("stack pointer restored", "r15=0x0000f000"),
    ("algorithm result is PASS", "r7 =0x00000000"),
]

def initialize():
    for stream in (sys.stdin, sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8")
        except Exception:
            pass

def run_test() -> bool:
    initialize()
    print("Running string-compare validation...\n")
    for path in ("test.script", "string-compare.bin"):
        if not os.path.exists(path):
            print(f"❌ Missing required file: {path}")
            return False
    try:
        with open("test.script", "r", encoding="utf-8") as script:
            result = subprocess.run(
                [VM], stdin=script, stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT, text=True, check=False,
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
    if not os.path.exists(LOG_FILE):
        print(f"❌ Missing log file: {LOG_FILE}")
        return False
    with open(LOG_FILE, "r", encoding="utf-8") as log_file:
        log = log_file.read().lower()
    passed = True
    for label, expected in CHECKS:
        if expected.lower() in log:
            print(f"✅ {label}")
        else:
            print(f"❌ {label}")
            print(f"   Missing: {expected}")
            passed = False
    print()
    print("PASS: string-compare" if passed else "FAIL: string-compare")
    return passed

if __name__ == "__main__":
    sys.exit(0 if run_test() else 1)
