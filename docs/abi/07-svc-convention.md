# 07 — SVC Convention

## 1. Status

This chapter reserves the application-facing system-service ABI.

The exact instruction-level SVC mechanism remains platform-defined until the architecture service model is finalized.

## 2. Proposed register assignment

```text
r0 = service number
r1 = argument 1
r2 = argument 2
r3 = argument 3
```

Return:

```text
r0 = result
r1 = optional secondary result or status
```

## 3. Preservation

An SVC behaves like a function call from the application's perspective.

The caller must assume:

```text
r0-r7 may change
r8-r14 are preserved
r15 is restored
```

## 4. Errors

A conventional signed return model is recommended:

```text
r0 >= 0  success result
r0 < 0   negative error code
```

This remains provisional.

## 5. Example concept

```asm
movi r0, SVC_CONSOLE_WRITE
mov  r1, buffer
mov  r2, length
trap
```

The exact mnemonic and dispatch mechanism must match the final architecture specification.

## 6. Relationship to BIOS

BIOS calls and operating-system SVC calls may share register conventions while remaining distinct interfaces.

The ABI should not require them to use the same dispatch table.
