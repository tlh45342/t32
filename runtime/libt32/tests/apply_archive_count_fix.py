#!/usr/bin/env python3
"""
Surgical hotfix for the stale libt32 archive-member-count regression test.

Expected use from the T32 repository root:

    python runtime/libt32/tests/apply_archive_count_fix.py

The script intentionally refuses to modify run_tests.py unless it finds the
known stale "sixteen members" expectation.  It creates a .bak copy first.
"""

from pathlib import Path
import re
import shutil
import sys

TEST_FILE = Path(__file__).with_name("run_tests.py")
BACKUP_FILE = TEST_FILE.with_suffix(".py.bak")

if not TEST_FILE.exists():
    print(f"ERROR: {TEST_FILE} not found", file=sys.stderr)
    raise SystemExit(2)

text = TEST_FILE.read_text(encoding="utf-8")
original = text

if "archive contains eighteen members" in text:
    print("libt32 test already expects eighteen archive members; no change needed.")
    raise SystemExit(0)

if "archive contains sixteen members" not in text:
    print(
        "ERROR: known stale diagnostic 'archive contains sixteen members' "
        "was not found; refusing to modify the file.",
        file=sys.stderr,
    )
    raise SystemExit(3)

# Update the human-readable test label.
text = text.replace(
    "archive contains sixteen members",
    "archive contains eighteen members",
    1,
)

# Patch the associated hard-coded member-count assertion.  The historical
# test has used a literal 16; accept common Python spellings but require
# exactly one count replacement near archive-member/list logic.
patterns = [
    (r"(len\s*\(\s*members\s*\)\s*==\s*)16\b", r"\g<1>18"),
    (r"(len\s*\(\s*archive_members\s*\)\s*==\s*)16\b", r"\g<1>18"),
    (r"(len\s*\(\s*member_lines\s*\)\s*==\s*)16\b", r"\g<1>18"),
    (r"(len\s*\(\s*lines\s*\)\s*==\s*)16\b", r"\g<1>18"),
    (r"(len\s*\([^)\n]*\)\s*==\s*)16\b", r"\g<1>18"),
]

count_changed = False
for pattern, replacement in patterns:
    new_text, n = re.subn(pattern, replacement, text, count=1)
    if n:
        text = new_text
        count_changed = True
        break

if not count_changed:
    # Some versions may compare against a named EXPECTED_* constant.
    # Try a narrowly-scoped archive/member constant.
    const_patterns = [
        (r"(?m)^(\s*EXPECTED_(?:ARCHIVE_)?MEMBERS\s*=\s*)16\s*$", r"\g<1>18"),
        (r"(?m)^(\s*ARCHIVE_MEMBER_COUNT\s*=\s*)16\s*$", r"\g<1>18"),
        (r"(?m)^(\s*EXPECTED_MEMBER_COUNT\s*=\s*)16\s*$", r"\g<1>18"),
    ]
    for pattern, replacement in const_patterns:
        new_text, n = re.subn(pattern, replacement, text, count=1)
        if n:
            text = new_text
            count_changed = True
            break

if not count_changed:
    print(
        "ERROR: found the stale label but could not identify the associated "
        "hard-coded count safely. No changes written.",
        file=sys.stderr,
    )
    raise SystemExit(4)

if text == original:
    print("No change needed.")
    raise SystemExit(0)

shutil.copy2(TEST_FILE, BACKUP_FILE)
TEST_FILE.write_text(text, encoding="utf-8", newline="\n")

print(f"Patched: {TEST_FILE}")
print(f"Backup : {BACKUP_FILE}")
print("Changed stale libt32 archive expectation: 16 -> 18")
print()
print("Next:")
print("  make -C runtime/libt32 test")
print("  do")
