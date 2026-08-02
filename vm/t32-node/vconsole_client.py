#!/usr/bin/env python3
"""Minimal vconsole JSON-over-TCP client for t32-node POC."""

from __future__ import annotations

import json
import socket
from typing import Iterable


def _send_request(host: str, port: int, msg: dict, timeout: float = 2.0) -> dict:
    data = json.dumps(msg, ensure_ascii=False, separators=(",", ":")) + "\n"
    with socket.create_connection((host, port), timeout=timeout) as sock:
        sock.sendall(data.encode("utf-8"))
        f = sock.makefile("r", encoding="utf-8", newline="\n")
        line = f.readline()
    if not line:
        return {"status": "error", "error": "no reply"}
    return json.loads(line)


def clear(host: str, port: int, session: str = "default") -> dict:
    return _send_request(host, port, {
        "session": session,
        "channel": "screen",
        "op": "clear",
        "fg": 7,
        "bg": 0,
    })


def put_text(host: str, port: int, x: int, y: int, text: str, session: str = "default") -> dict:
    return _send_request(host, port, {
        "session": session,
        "channel": "screen",
        "op": "put_text",
        "x": x,
        "y": y,
        "text": text,
        "fg": 7,
        "bg": 0,
    })


def publish_lines(host: str, port: int, lines: Iterable[str], session: str = "default") -> None:
    clear(host, port, session=session)
    for y, line in enumerate(lines):
        if y >= 25:
            break
        if line:
            put_text(host, port, 0, y, line[:80], session=session)
