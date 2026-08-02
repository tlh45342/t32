#!/usr/bin/env python3
"""
t32core.py - tiny t32 proof-of-concept CPU/runtime.

This is intentionally small and dumb:
- fixed 32-bit instruction words
- MOVI/ADDI/SUBI/JMP/JZ/JNZ consume a second 32-bit immediate word
- flat RAM loaded at address 0
- 0x90003000..0x90003F9F is an 80x25 text video buffer
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import List, Optional, TextIO

VIDEO_BASE = 0x90003000
VIDEO_COLS = 80
VIDEO_ROWS = 25
VIDEO_SIZE = VIDEO_COLS * VIDEO_ROWS * 2

OP_HALT = 0x00
OP_MOV  = 0x01
OP_MOVI = 0x02
OP_ADD  = 0x03
OP_ADDI = 0x04
OP_SUB  = 0x05
OP_SUBI = 0x06
OP_LDB  = 0x07
OP_STB  = 0x08
OP_JMP  = 0x09
OP_JZ   = 0x0A
OP_JNZ  = 0x0B

OP_NAMES = {
    OP_HALT: "HALT",
    OP_MOV: "MOV",
    OP_MOVI: "MOVI",
    OP_ADD: "ADD",
    OP_ADDI: "ADDI",
    OP_SUB: "SUB",
    OP_SUBI: "SUBI",
    OP_LDB: "LDB",
    OP_STB: "STB",
    OP_JMP: "JMP",
    OP_JZ: "JZ",
    OP_JNZ: "JNZ",
}


def u32(value: int) -> int:
    return value & 0xFFFFFFFF


@dataclass
class T32Machine:
    ram_size: int = 1024 * 1024
    regs: List[int] = field(default_factory=lambda: [0] * 16)
    pc: int = 0
    halted: bool = False
    ram: bytearray = field(init=False)
    video: bytearray = field(default_factory=lambda: bytearray(VIDEO_SIZE))
    steps: int = 0

    def __post_init__(self) -> None:
        self.ram = bytearray(self.ram_size)

    def load_binary(self, path: str, base: int = 0) -> int:
        with open(path, "rb") as f:
            data = f.read()
        if base < 0 or base + len(data) > len(self.ram):
            raise RuntimeError(f"binary does not fit in RAM: {len(data)} bytes at 0x{base:08X}")
        self.ram[base:base + len(data)] = data
        self.pc = base
        return len(data)

    def fetch_u32(self) -> int:
        if self.pc + 4 > len(self.ram):
            raise RuntimeError(f"PC out of RAM: 0x{self.pc:08X}")
        value = int.from_bytes(self.ram[self.pc:self.pc + 4], "little")
        self.pc = u32(self.pc + 4)
        return value

    def read_u8(self, addr: int) -> int:
        addr = u32(addr)
        if VIDEO_BASE <= addr < VIDEO_BASE + VIDEO_SIZE:
            return self.video[addr - VIDEO_BASE]
        if addr < len(self.ram):
            return self.ram[addr]
        raise RuntimeError(f"read_u8 unmapped address: 0x{addr:08X}")

    def write_u8(self, addr: int, value: int) -> None:
        addr = u32(addr)
        value &= 0xFF
        if VIDEO_BASE <= addr < VIDEO_BASE + VIDEO_SIZE:
            self.video[addr - VIDEO_BASE] = value
            return
        if addr < len(self.ram):
            self.ram[addr] = value
            return
        raise RuntimeError(f"write_u8 unmapped address: 0x{addr:08X}")

    def decode_fields(self, instr: int) -> tuple[int, int, int, int]:
        op = (instr >> 24) & 0xFF
        rd = (instr >> 20) & 0x0F
        ra = (instr >> 16) & 0x0F
        rb = (instr >> 12) & 0x0F
        return op, rd, ra, rb

    def trace_regs(self) -> str:
        return " ".join(f"r{i}={self.regs[i]:08X}" for i in range(8))

    def step(self, trace: Optional[TextIO] = None) -> None:
        if self.halted:
            return

        pc0 = self.pc
        instr = self.fetch_u32()
        op, rd, ra, rb = self.decode_fields(instr)
        name = OP_NAMES.get(op, f"OP_{op:02X}")
        detail = ""

        if op == OP_HALT:
            self.halted = True
            detail = "halt"

        elif op == OP_MOV:
            self.regs[rd] = self.regs[ra]
            detail = f"r{rd}=r{ra} ; {self.regs[rd]:08X}"

        elif op == OP_MOVI:
            imm = self.fetch_u32()
            self.regs[rd] = imm
            detail = f"r{rd}=0x{imm:08X}"

        elif op == OP_ADD:
            self.regs[rd] = u32(self.regs[ra] + self.regs[rb])
            detail = f"r{rd}=r{ra}+r{rb} ; {self.regs[rd]:08X}"

        elif op == OP_ADDI:
            imm = self.fetch_u32()
            self.regs[rd] = u32(self.regs[ra] + imm)
            detail = f"r{rd}=r{ra}+0x{imm:08X} ; {self.regs[rd]:08X}"

        elif op == OP_SUB:
            self.regs[rd] = u32(self.regs[ra] - self.regs[rb])
            detail = f"r{rd}=r{ra}-r{rb} ; {self.regs[rd]:08X}"

        elif op == OP_SUBI:
            imm = self.fetch_u32()
            self.regs[rd] = u32(self.regs[ra] - imm)
            detail = f"r{rd}=r{ra}-0x{imm:08X} ; {self.regs[rd]:08X}"

        elif op == OP_LDB:
            addr = self.regs[ra]
            self.regs[rd] = self.read_u8(addr)
            detail = f"r{rd}=mem8[0x{addr:08X}] ; {self.regs[rd]:02X}"

        elif op == OP_STB:
            addr = self.regs[ra]
            self.write_u8(addr, self.regs[rb])
            detail = f"mem8[0x{addr:08X}]=r{rb} ; {self.regs[rb] & 0xFF:02X}"

        elif op == OP_JMP:
            target = self.fetch_u32()
            self.pc = target
            detail = f"pc=0x{target:08X}"

        elif op == OP_JZ:
            target = self.fetch_u32()
            taken = self.regs[ra] == 0
            if taken:
                self.pc = target
            detail = f"if r{ra}==0 goto 0x{target:08X} ; {'taken' if taken else 'not-taken'}"

        elif op == OP_JNZ:
            target = self.fetch_u32()
            taken = self.regs[ra] != 0
            if taken:
                self.pc = target
            detail = f"if r{ra}!=0 goto 0x{target:08X} ; {'taken' if taken else 'not-taken'}"

        else:
            raise RuntimeError(f"unknown opcode 0x{op:02X} at PC=0x{pc0:08X}")

        self.steps += 1
        if trace:
            trace.write(f"{self.steps:08d} pc={pc0:08X} instr={instr:08X} {name:<5} {detail} | {self.trace_regs()}\n")

    def run(self, max_steps: int = 1_000_000, trace: Optional[TextIO] = None) -> int:
        while not self.halted and self.steps < max_steps:
            self.step(trace=trace)
        if not self.halted:
            raise RuntimeError(f"max steps exceeded: {max_steps}")
        return self.steps

    def video_lines(self) -> list[str]:
        lines: list[str] = []
        for y in range(VIDEO_ROWS):
            chars = []
            for x in range(VIDEO_COLS):
                b = self.video[(y * VIDEO_COLS + x) * 2]
                if b == 0:
                    chars.append(" ")
                elif 32 <= b <= 126:
                    chars.append(chr(b))
                else:
                    chars.append(".")
            lines.append("".join(chars).rstrip())
        return lines

    def dump_screen(self) -> str:
        return "\n".join(self.video_lines())
