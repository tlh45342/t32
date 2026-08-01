# libt32 routine ABI — draft 0.0.1

This source library follows the register contracts written at the top of each
routine. T32 does not yet have a finalized link-time ABI.

Current working rules:

- `r15` is the stack pointer and must be restored before return.
- `CALL` and `RET` use the machine stack.
- Each routine documents its input, output, and clobbered registers.
- Test programs should not preserve values in registers listed as clobbered.
- No routine currently assumes flag-consuming conditional branches.
- `JZ` and `JNZ` always name the register they test.

The object-format phase must formalize:

- global and local symbol visibility;
- caller-saved and callee-saved registers;
- section placement and alignment;
- relocation types;
- archive member selection.
