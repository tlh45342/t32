from __future__ import annotations
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
manifest = json.loads((ROOT / "manifest.json").read_text(encoding="utf-8"))
missing = [name for name in manifest["routines"] if not (ROOT / name).is_file()]

print(f'Checking {manifest["name"]} {manifest["version"]}...')
print(f'  state: {manifest["state"]}')
print(f'  routines: {len(manifest["routines"])}')

if missing:
    for name in missing:
        print(f"  FAIL missing {name}")
    sys.exit(1)

for name in manifest["routines"]:
    text = (ROOT / name).read_text(encoding="utf-8")
    if "\t" in text:
        print(f"  FAIL tab found in {name}")
        sys.exit(1)
    if not text.endswith("\n"):
        print(f"  FAIL no trailing newline in {name}")
        sys.exit(1)

print("libt32: PASS")
