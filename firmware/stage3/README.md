# T32 C Stage 3 0.0.12

Stage3 is now the first usable interactive T32 monitor.

## Input editing milestone

The command line now supports printable ASCII echo, Enter to submit a line,
blank Enter to reprompt, Backspace/DEL to remove the previous character from
both the input buffer and visible console, and a fixed 63-character command
limit.

The parser keeps a single line length/cursor index so Left/Right/Home/End can
be added later without replacing the command dispatcher.

Current commands:

```text
help
version
bootinfo
mem
halt
```

The full path remains:

```text
BIOS -> BOOT.BIN -> NEXT.BIN / Stage3 -> C monitor -> libt32 console
```

This release also extends `libt32 putchar` with destructive Backspace behavior,
keeping terminal mechanics in the runtime instead of hard-coding framebuffer
editing into Stage3.
