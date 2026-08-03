from __future__ import annotations

import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "build"
SOURCE = ROOT / "src" / "crt0.s"
OBJECT = BUILD / "crt0.o"


def command(name: str) -> str:
    found = shutil.which(name)
    if not found:
        raise SystemExit(f"crt0: required tool not found in PATH: {name}")
    return found


def run(args: list[str]) -> None:
    print(" ".join(args))
    result = subprocess.run(args, cwd=ROOT, check=False)
    if result.returncode != 0:
        raise SystemExit(result.returncode)


def build() -> None:
    BUILD.mkdir(parents=True, exist_ok=True)
    run([command("t32-as"), "-f", "obj", str(SOURCE), "-o", str(OBJECT)])
    print(f"crt0: created {OBJECT}")


def clean() -> None:
    if BUILD.exists():
        shutil.rmtree(BUILD)
    print("crt0: cleaned")


def install() -> None:
    build()

    if os.name == "nt":
        prefix = Path(os.environ.get("USERPROFILE", str(Path.home()))) / ".local"
    else:
        prefix = Path.home() / ".local"

    destination = prefix / "lib" / "t32"
    destination.mkdir(parents=True, exist_ok=True)
    shutil.copy2(OBJECT, destination / "crt0.o")
    print(f"Installed T32 startup object to {destination / 'crt0.o'}")


def main() -> int:
    action = sys.argv[1] if len(sys.argv) > 1 else "build"

    if action == "build":
        build()
    elif action == "clean":
        clean()
    elif action == "install":
        install()
    else:
        print("usage: python tools/build.py [build|clean|install]", file=sys.stderr)
        return 2

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
