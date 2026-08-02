# libt32 routine ABI — working object-library contract

`libt32 0.0.2` preserves the register contracts proven by the algorithm suite.
Each source file documents its arguments, result registers, and clobbers.

Current common rules:

- `r15` is the stack pointer and must be restored before return.
- `CALL` and `RET` use the T32 machine stack.
- `JZ` and `JNZ` test an explicitly named register.
- A caller may preserve values only in registers not listed as clobbered by the routine.
- Every public routine is exported with `.global` from `.text`.
- Internal loop and data labels remain local to their object file.

A single compiler-facing calling convention has not yet replaced the
routine-specific historical contracts. That ABI consolidation should happen
before `t32-cc` begins generating general library calls.
