# 10 — Design Rationale

## 1. Why four argument registers?

Four registers cover many small systems routines without forcing immediate stack traffic.

They also keep early compiler code generation simple.

## 2. Why return in `r0`?

Using the first argument register as the primary return register is conventional, compact, and compiler-friendly.

It allows simple functions such as identity operations to return without moving data.

## 3. Why caller-saved `r0-r7`?

Temporary registers are commonly used for expression evaluation and short-lived values.

Making them caller-saved reduces prologue and epilogue work for small functions.

## 4. Why callee-saved `r8-r14`?

Long-lived values, frame pointers, and register-allocated locals need stable storage across calls.

A block of preserved registers gives compilers and assembly programmers that option.

## 5. Why is `r15` the stack pointer?

The project already uses `r15` as the stack pointer.

Keeping the convention stable avoids unnecessary instruction-set and toolchain changes.

## 6. Why no dedicated frame pointer?

Many small functions do not need one.

Reserving a frame pointer permanently would reduce the available register set.

A compiler may select a callee-saved register when a stable frame base is useful.

## 7. Why caller stack cleanup?

Caller cleanup supports:

- fixed and variable argument counts;
- cdecl-like behavior;
- simple callee return sequences.

## 8. Why 4-byte alignment?

T32 is a 32-bit machine.

Four-byte alignment is sufficient for the initial scalar and pointer model and avoids unnecessary padding.

## 9. Why keep the ABI draft?

The current `libt32` routines were originally written as algorithm tests and use mixed return registers.

The ABI should guide the refactor rather than falsely claim the existing code already conforms.

## 10. Why document rationale?

Rules without rationale become folklore.

Rationale preserves the design path and lets future maintainers distinguish intentional constraints from temporary implementation limits.
