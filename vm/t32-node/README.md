# t32-node POC

First proof-of-concept t32 runtime.

It runs the current raw `t32-asm` output format:

- fixed 32-bit instruction words
- little-endian words
- `MOVI`, `ADDI`, `SUBI`, `JMP`, `JZ`, `JNZ` consume a second 32-bit immediate word
- flat RAM loaded at address `0x00000000`
- memory-mapped text video at `0x90003000`

## Run debug locally

```bash
python t32-node-dbg.py samples/hello.t32 --dump-screen --trace hello.trace
```

You should see `Hello World` in the dumped video screen.

## Publish final screen to vconsole

```bash
python t32-node-dbg.py samples/hello.t32 --vconsole-host 192.168.160.73 --vconsole-port 8765 --dump-screen
```

or the quieter runner:

```bash
python t32-node.py samples/hello.t32 --vconsole-host 192.168.160.73 --vconsole-port 8765
```

## Docker

```bash
docker compose up --build
```

This POC intentionally publishes the final screen only after HALT. Later versions can refresh periodically or on dirty video pages.
