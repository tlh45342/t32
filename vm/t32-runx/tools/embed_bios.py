#!/usr/bin/env python3
from pathlib import Path
import sys

def main() -> int:
    if len(sys.argv) != 3:
        print("usage: embed_bios.py <bios.bin> <output.h>")
        return 2

    source = Path(sys.argv[1])
    output = Path(sys.argv[2])
    data = source.read_bytes()

    output.parent.mkdir(parents=True, exist_ok=True)

    lines = [
        "/* generated file: do not edit */",
        "#ifndef T32_RUNX_DEFAULT_BIOS_H",
        "#define T32_RUNX_DEFAULT_BIOS_H",
        "",
        "#include <stddef.h>",
        "#include <stdint.h>",
        "",
        "static const uint8_t t32_runx_default_bios[] = {",
    ]

    for offset in range(0, len(data), 12):
        chunk = data[offset:offset + 12]
        lines.append("    " + ", ".join(f"0x{b:02x}" for b in chunk) + ",")

    lines += [
        "};",
        "",
        "static const size_t t32_runx_default_bios_size =",
        "    sizeof(t32_runx_default_bios);",
        "",
        "#endif",
        "",
    ]

    output.write_text("\n".join(lines), encoding="utf-8", newline="\n")
    print(f"embedded {len(data)} BIOS bytes -> {output}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
