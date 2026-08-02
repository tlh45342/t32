#!/usr/bin/env python3
"""t32-node-dbg - visible/debug POC runner for t32 binaries."""

from __future__ import annotations

import argparse
import sys

from t32core import T32Machine
from vconsole_client import publish_lines


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(description="t32 debug runner")
    p.add_argument("binary", help="raw t32 binary assembled by t32-asm")
    p.add_argument("--trace", help="write execution trace to file")
    p.add_argument("--max-steps", type=int, default=1_000_000)
    p.add_argument("--ram", type=lambda s: int(s, 0), default=1024 * 1024)
    p.add_argument("--dump-screen", action="store_true", help="print final 80x25 text screen")
    p.add_argument("--vconsole-host", default=None, help="publish final screen to vconsole host")
    p.add_argument("--vconsole-port", type=int, default=8765)
    p.add_argument("--session", default="default")
    args = p.parse_args(argv)

    m = T32Machine(ram_size=args.ram)
    n = m.load_binary(args.binary)
    print(f"loaded {n} bytes at 0x00000000")

    trace_file = open(args.trace, "w", encoding="utf-8") if args.trace else sys.stdout
    close_trace = bool(args.trace)
    try:
        steps = m.run(max_steps=args.max_steps, trace=trace_file)
    finally:
        if close_trace:
            trace_file.close()

    print(f"halted after {steps} steps")

    if args.dump_screen:
        print("----- t32 video -----")
        print(m.dump_screen())
        print("---------------------")

    if args.vconsole_host:
        publish_lines(args.vconsole_host, args.vconsole_port, m.video_lines(), session=args.session)
        print(f"published video to {args.vconsole_host}:{args.vconsole_port} session={args.session}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
