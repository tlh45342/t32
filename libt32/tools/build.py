from __future__ import annotations

import argparse
import json
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "build"
MANIFEST = ROOT / "manifest.json"
ARCHIVE = BUILD / "libt32.a"


def run(command: list[str]) -> None:
    print(" ".join(command))
    result = subprocess.run(command, cwd=ROOT, check=False)
    if result.returncode != 0:
        raise SystemExit(result.returncode)


def object_path(source: Path) -> Path:
    relative = source.relative_to(ROOT / "src")
    return BUILD / relative.with_suffix(".o")


def sources() -> list[Path]:
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    return [ROOT / item for item in manifest["routines"]]


def objects() -> list[Path]:
    return [object_path(source) for source in sources()]


def build_objects() -> None:
    for source in sources():
        target = object_path(source)
        target.parent.mkdir(parents=True, exist_ok=True)
        run(["t32-as", "-f", "obj", str(source), "-o", str(target)])
    print(f"libt32: built {len(sources())} objects")


def build_archive() -> None:
    build_objects()
    ARCHIVE.parent.mkdir(parents=True, exist_ok=True)
    if ARCHIVE.exists():
        ARCHIVE.unlink()
    run(["t32-ar", "rcs", str(ARCHIVE), *[str(item) for item in objects()]])
    print(f"libt32: created {ARCHIVE}")


def inspect_objects() -> None:
    build_archive()
    representatives = [
        BUILD / "memory" / "memmove.o",
        BUILD / "string" / "strlen.o",
        BUILD / "convert" / "hex_to_string.o",
    ]
    for item in representatives:
        print()
        run(["t32-nm", str(item)])
    print()
    run(["t32-ar", "t", str(ARCHIVE)])


def clean() -> None:
    if BUILD.exists():
        shutil.rmtree(BUILD)
    BUILD.mkdir(parents=True, exist_ok=True)
    print("libt32: cleaned")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("command", choices=["objects", "archive", "inspect", "clean"])
    args = parser.parse_args()
    if args.command == "objects":
        build_objects()
    elif args.command == "archive":
        build_archive()
    elif args.command == "inspect":
        inspect_objects()
    else:
        clean()
    return 0


if __name__ == "__main__":
    sys.exit(main())
