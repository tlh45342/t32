# T32 Application Binary Interface

**Status:** Draft  
**Version:** 0.1  
**Date:** 2026  
**Reference implementations:** `t32-as`, `t32-ld`, `t32-run`, `libt32`

## Purpose

The T32 ABI defines the binary contract between independently written software components.

It specifies:

- register roles;
- argument passing;
- return values;
- caller-saved and callee-saved state;
- stack organization;
- function entry and exit;
- program entry;
- runtime startup;
- object-symbol conventions;
- system-service calling conventions;
- interrupt entry and return;
- library routine documentation.

A conforming T32 compiler, assembler module, linker, runtime library, operating system component, and application must follow the same ABI rules when they exchange control or data.

## Why this exists

A machine can execute instructions without an ABI.

A software ecosystem cannot.

The ABI allows code produced at different times, by different tools, and by different authors to interoperate without knowing how the other component was implemented.

## Reading order

```text
01-register-convention.md
02-calling-convention.md
03-stack-frame.md
04-process-entry.md
05-object-symbols.md
06-runtime-startup.md
07-svc-convention.md
08-interrupt-convention.md
09-library-convention.md
10-design-rationale.md
```

## Draft status

Version 0.1 is intentionally provisional.

The ABI should remain draft until:

- `libt32` conforms to it;
- `t32-cc` emits code that conforms to it;
- stack and register-preservation tests pass;
- `crt0` conforms to it;
- representative linked C programs execute successfully.

At that point the ABI may be promoted to version 1.0.
