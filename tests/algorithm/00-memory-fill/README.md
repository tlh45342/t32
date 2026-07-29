# 00-memory-fill

Validates a foundational byte-oriented memory-fill algorithm.

## Behavior

The guest program:

1. Fills 16 bytes beginning at `0x00009000` with `0xA5`.
2. Reads every byte back.
3. Compares each byte with the expected value.
4. Places the result in `r7`.
5. Executes `HALT`.

`r7 == 0` means PASS. Any nonzero value means FAIL.

## Independent validation

The test intentionally uses two validation layers:

- Guest-side verification checks the completed buffer using T32 instructions.
- Host-side validation checks registers, halt state, and the memory dump.

This avoids treating the guest PASS register as the only source of truth.

## Run

```text
make test
```

## Expected final state

```text
r0 = 0x00009000
r1 = 0x00009010
r2 = 0x000000A5
r3 = 0x00000000
r4 = 0x00009010
r5 = 0x00000000
r6 = 0x00000000
r7 = 0x00000000
state = halted
reason = HALT instruction
```
