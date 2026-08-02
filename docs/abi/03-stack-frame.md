# 03 — Stack Frame

## 1. Growth direction

The T32 stack grows toward lower addresses.

Conceptually:

```text
Higher addresses

+-----------------------------+
| caller-owned stack arguments|
+-----------------------------+
| return address              |
+-----------------------------+
| saved callee registers      |
+-----------------------------+
| local storage               |
+-----------------------------+
| temporary spill storage     |
+-----------------------------+
r15 -> current stack top

Lower addresses
```

## 2. Alignment

The stack shall be aligned to 4 bytes at every public function-call boundary.

A compiler may use stricter alignment for a specific object, but may not assume stricter alignment from external callers unless a later ABI version defines it.

## 3. Frame pointer

Version 0.1 does not reserve a dedicated frame-pointer register.

A compiler may use one of the callee-saved registers, such as `r14`, as a frame pointer when useful.

When used, it must be restored before return.

## 4. Minimal frame

```asm
function:
    push r8
    push r9

    ; function body

    pop  r9
    pop  r8
    ret
```

Registers are restored in reverse order.

## 5. Locals

Local stack storage is reserved by decreasing `r15`.

Conceptually:

```asm
subi r15, r15, 16
```

and released by:

```asm
addi r15, r15, 16
```

A function must not return while local stack storage remains allocated.

## 6. Stack arguments

The exact offset of an incoming stack argument depends on:

- the return-address representation used by `CALL`;
- registers saved by the callee;
- local storage.

Compilers and assembly routines must calculate those offsets consistently with the machine's `CALL` behavior.

The first ABI validation suite should explicitly test stack-argument offsets.

## 7. No red zone

Version 0.1 defines no red zone.

Memory below `r15` is not available until the function explicitly allocates it.

## 8. Interrupt interaction

An interrupt handler may use the current stack only according to the interrupt ABI.

Ordinary functions must not assume interrupts preserve caller-saved registers unless the interrupt entry mechanism guarantees it.
