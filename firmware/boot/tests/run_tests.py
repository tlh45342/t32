from __future__ import annotations

import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
WORK = ROOT / "tests" / "build"


def req(condition: bool, message: str, output: str = "") -> None:
    if not condition:
        print("  FAIL", message)
        if output:
            print(output)
        raise SystemExit(1)
    print("  PASS", message)


def run_boot(runner: str, image: Path, bootinfo: Path, *, pointer: int = 0x2000) -> str:
    script = (
        f"load {bootinfo} 0x2000\n"
        f"load {image} 0x10000\n"
        f"set r0 0x{pointer:x}\n"
        "set pc 0x10000\n"
        "run\n"
        "display\n"
        "quit\n"
    )
    result = subprocess.run(
        [runner],
        input=script,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        cwd=ROOT,
        check=False,
    )
    req(result.returncode == 0, "standalone BOOT.BIN monitor execution succeeds", result.stdout)
    return result.stdout


def write_bootinfo(path: Path, magic: int = 0x42323354) -> None:
    words = (
        magic,          # +00 magic T32B
        72,             # +04 structure size
        0,              # +08 version major
        2,              # +0c version minor
        1,              # +10 flags: boot disk valid
        0,              # +14 RAM base
        1024 * 1024,    # +18 RAM size
        0,              # +1c boot disk
        512,            # +20 sector size
        2048,           # +24 sector count
        0x90000000,     # +28 text framebuffer
        80,             # +2c columns
        25,             # +30 rows
        0x90001000,     # +34 disk MMIO
        0x90004000,     # +38 platform MMIO
        0x00010000,     # +3c boot entry
        0,              # +40 BIOS service ABI absent in standalone test
        0,              # +44 disk_read service absent
    )
    path.write_bytes(struct.pack("<18I", *words))


def main() -> int:
    if len(sys.argv) != 3:
        print("usage: run_tests.py <t32-run> <boot.bin>")
        return 2

    runner = sys.argv[1]
    image = Path(sys.argv[2]).resolve()

    print("Running T32 BOOT 0.0.4 validation...")
    req(image.exists(), "boot.bin exists")
    req(image.stat().st_size > 0, "boot.bin is non-empty")

    WORK.mkdir(parents=True, exist_ok=True)
    valid = WORK / "bootinfo.bin"
    invalid = WORK / "bootinfo-invalid.bin"
    write_bootinfo(valid)
    write_bootinfo(invalid, magic=0)
    req(valid.stat().st_size == 72, "synthetic Bootinfo v0.2 is 72 bytes")

    output = run_boot(runner, image, valid)
    req("T32 BOOT 0.0.4" in output, "BOOT banner appears", output)
    req("BOOTINFO v0.2 OK" in output, "BOOT accepts valid Bootinfo v0.2", output)
    req("Hello from BOOT.BIN" in output, "Hello payload appears", output)

    bad_output = run_boot(runner, image, invalid)
    req("BOOTINFO v0.2 INVALID" in bad_output, "BOOT rejects invalid Bootinfo", bad_output)

    print("t32-boot: PASS (8/8 cases)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
