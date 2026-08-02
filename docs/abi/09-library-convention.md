# 09 — Library Convention

## 1. Purpose

Every public `libt32` routine should document the same contract.

Required headings:

```text
Purpose
Arguments
Returns
Preserves
Clobbers
Notes
```

## 2. Example source header

```asm
;
; strlen
;
; Purpose
;     Return the length of a zero-terminated byte string.
;
; Arguments
;     r0 = pointer to string
;
; Returns
;     r0 = length in bytes
;
; Preserves
;     r8-r15
;
; Clobbers
;     r1-r2
;
; Notes
;     The terminating zero is not included.
;

.section .text
.global strlen
```

## 3. ABI conformance

Public library routines must:

- return primary results in `r0`;
- preserve `r8-r14`;
- restore `r15`;
- use only documented stack arguments;
- export exactly the intended global symbols.

## 4. Internal helpers

Internal helpers should remain local unless another object file must call them.

## 5. Standard names

C-compatible routines should use conventional names:

```text
memcpy
memmove
memset
memcmp
memchr
strlen
strcpy
strncpy
strcmp
strncmp
strchr
strstr
```

## 6. Nonstandard names

Extensions should be clearly identified as nonstandard:

```text
strrev
hex_to_string
string_to_hex
```

## 7. Testing

Each public routine should eventually have:

- an algorithm-level correctness test;
- an ABI conformance test;
- a linked archive integration test where appropriate.
