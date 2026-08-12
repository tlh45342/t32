from __future__ import annotations

import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
WORK = ROOT / "tests" / "build"


def passed(message: str) -> None:
    print(f"  PASS {message}")


def require(condition: bool, message: str, output: str = "") -> None:
    if not condition:
        print(f"  FAIL {message}")
        if output:
            print(output)
        raise SystemExit(1)
    passed(message)


def run_process(argv: list[str], *, stdin: str = "", cwd: Path = ROOT) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        argv,
        input=stdin,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        cwd=cwd,
        check=False,
        timeout=15,
    )


def run_monitor(runner: Path, bios: Path, disk: Path | None, keys: str | None = None) -> str:
    commands: list[str] = []
    if disk is not None:
        commands.append(f"disk attach {disk}")
    commands += [
        f"load {bios} 0x1000",
        "set pc 0x1000",
    ]
    if keys is not None:
        commands.append(f"key {keys}")
    commands += [
        "run",
        "display",
        "e 0x2000 72",
        "quit",
    ]
    result = run_process([str(runner)], stdin="\n".join(commands) + "\n")
    require(result.returncode == 0, "monitor execution exits successfully", result.stdout)
    return result.stdout


def main() -> int:
    if len(sys.argv) != 6:
        print("usage: run_tests.py <t32-run> <bios.bin> <boot.bin> <next.bin> <t32-disk>")
        return 2

    runner = Path(sys.argv[1]).resolve()
    bios = Path(sys.argv[2]).resolve()
    boot = Path(sys.argv[3]).resolve()
    next_image = Path(sys.argv[4]).resolve()
    disk_tool = Path(sys.argv[5]).resolve()

    print("Running T32 BIOS 0.0.6 validation...")
    require(runner.is_file(), "t32-run exists")
    require(disk_tool.is_file(), "t32-disk exists")
    require(bios.is_file() and bios.stat().st_size > 0, "bios.bin exists and is non-empty")
    require(boot.is_file() and boot.stat().st_size > 0, "BOOT.BIN payload exists and is non-empty")
    require(next_image.is_file() and next_image.stat().st_size > 0, "NEXT.BIN payload exists and is non-empty")

    shutil.rmtree(WORK, ignore_errors=True)
    WORK.mkdir(parents=True)

    no_disk = run_monitor(runner, bios, None)
    require("T32 BIOS 0.0.6" in no_disk, "BIOS banner appears without media", no_disk)
    require("Disk0 not present" in no_disk, "BIOS reports missing disk0", no_disk)

    raw_disk = WORK / "raw.img"
    raw_disk.write_bytes(bytes(512 * 16))
    bad_format = run_monitor(runner, bios, raw_disk)
    require("Disk0 not T32D v0.1" in bad_format, "BIOS rejects an unformatted disk", bad_format)

    boot_copy = WORK / "boot.bin"
    next_copy = WORK / "next.bin"
    shutil.copyfile(boot, boot_copy)
    shutil.copyfile(next_image, next_copy)

    boot_only_disk = WORK / "boot-only.img"
    boot_only_script = "\n".join([
        f"create {boot_only_disk} 1M",
        f"format {boot_only_disk}",
        f"put {boot_only_disk} {boot_copy} BOOT.BIN",
        "",
    ])
    boot_only_result = run_process([str(disk_tool)], stdin=boot_only_script, cwd=WORK)
    require(boot_only_result.returncode == 0, "BOOT-only T32D image is created", boot_only_result.stdout)
    boot_only = run_monitor(runner, bios, boot_only_disk)
    require("NEXT.BIN NOT FOUND" in boot_only, "BOOT reports missing NEXT.BIN", boot_only)

    boot_disk = WORK / "boot.img"
    disk_script = "\n".join([
        f"create {boot_disk} 1M",
        f"format {boot_disk}",
        f"put {boot_disk} {boot_copy} BOOT.BIN",
        f"put {boot_disk} {next_copy} NEXT.BIN",
        "",
    ])
    disk_result = run_process([str(disk_tool)], stdin=disk_script, cwd=WORK)
    require(disk_result.returncode == 0, "T32D boot image is created", disk_result.stdout)
    require("BOOT.BIN" in disk_result.stdout, "BOOT.BIN is installed into T32D image", disk_result.stdout)
    require("NEXT.BIN" in disk_result.stdout, "NEXT.BIN is installed into T32D image", disk_result.stdout)

    boot_trace = run_monitor(
        runner,
        bios,
        boot_disk,
        r"halt\r",
    )
    require("T32 BIOS 0.0.6" in boot_trace,
            "BIOS identification survives successful boot trace", boot_trace)

    booted = run_monitor(
        runner,
        bios,
        boot_disk,
        r"version\rhelp\rmem\rtime\rbootinfo\rhalt\r",
    )
    require("T32 Stage3 Monitor 0.0.13" in booted, "BOOT transfers control to NEXT.BIN", booted)
    require("help     show commands" in booted, "compiler-built third stage executes", booted)
    require("T32 Stage3 Monitor 0.0.13" in booted, "interactive C monitor starts", booted)
    require("help     show commands" in booted, "Stage3 help command executes through full boot chain", booted)
    require("Stage3 load address 0x00020000" in booted, "Stage3 mem command executes through full boot chain", booted)
    match = re.search(r"RTC epoch:\s*([0-9]+)", booted)
    require(match is not None, "Stage3 time command executes through full boot chain", booted)
    if match is not None:
        epoch = int(match.group(1))
        require(abs(epoch - int(__import__("time").time())) <= 10,
                "Stage3 RTC tracks host UTC seconds through full boot chain", booted)
    require("Bootinfo v0.2 handoff OK" in booted, "Stage3 bootinfo command executes through full boot chain", booted)
    require("Unknown command" not in booted, "scripted Stage3 commands are parsed cleanly", booted)
    require("54 33 32 42" in booted, "Bootinfo memory carries T32B magic", booted)
    require("48 00 00 00" in booted, "Bootinfo memory records 72-byte size", booted)
    require("00 00 10 00" in booted, "Bootinfo reports 1 MiB RAM", booted)
    require("00 02 00 00" in booted, "Bootinfo reports 512-byte sectors", booted)
    require("01 00 00 00 08 10 00 00" in booted,
            "Bootinfo publishes BIOS service v0.1 and disk_read entry", booted)

    print("t32-bios: PASS (35/35 cases)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
