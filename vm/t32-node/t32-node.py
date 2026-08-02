#!/usr/bin/env python3
"""t32-node - quiet/fast-ish POC runner for t32 binaries."""

from __future__ import annotations

import argparse

from t32core import T32Machine
from vconsole_client import publish_lines


def main() -> int:
    p = argparse.ArgumentParser(description="t32 POC runner")
    p.add_argument("binary")
    p.add_argument("--max-steps", type=int, default=1_000_000)
    p.add_argument("--ram", type=lambda s: int(s, 0), default=1024 * 1024)
    p.add_argument("--vconsole-host", default=None)
    p.add_argument("--vconsole-port", type=int, default=8765)
    p.add_argument("--session", default="default")
    args = p.parse_args()

    m = T32Machine(ram_size=args.ram)
    m.load_binary(args.binary)
    m.run(max_steps=args.max_steps, trace=None)

    if args.vconsole_host:
        publish_lines(args.vconsole_host, args.vconsole_port, m.video_lines(), session=args.session)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
