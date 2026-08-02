# 08 — Interrupt Convention

## 1. Purpose

The interrupt ABI defines the state visible to an interrupt handler and the state that must be restored before `IRET`.

## 2. Required preservation

A transparent interrupt must not change the interrupted program's observable register state unless the architecture explicitly defines otherwise.

The interrupt entry path must preserve enough state to restore:

- program counter;
- status or condition state, when present;
- `r0-r15`;
- interrupt-enable state;
- exception cause information needed by the handler.

## 3. Handler model

Conceptually:

```text
interrupt accepted
        ↓
machine or entry stub saves state
        ↓
handler executes
        ↓
state restored
        ↓
IRET
```

## 4. Interrupt stack

The platform must define whether interrupts use:

- the current program stack;
- a dedicated interrupt stack;
- a privilege-level stack.

Version 0.1 does not require one choice, but every platform must document it.

## 5. Nested interrupts

Nested interrupts are not required by ABI 0.1.

A handler may begin with interrupts disabled.

## 6. C handlers

A future C interrupt-handler attribute may generate the required save/restore sequence automatically.

Ordinary C functions are not interrupt handlers unless wrapped by a conforming entry stub.
