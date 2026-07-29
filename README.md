# T32 control-flow validation 0.0.1

This package adds seven cross-instruction control-flow validations:

- countdown loop;
- JZ taken;
- JZ not taken;
- JNZ taken;
- JNZ not taken;
- forward JMP;
- backward JMP.

The tests use ordered checkpoints where useful to prove exact PC movement and
branch-path selection, not merely final register values.

## Installation

Extract into the root of the `t32` repository.

If a prior `tools/t32_validation.py` exists, this compatible copy may replace it.

Run:

    make -C validation test

When `t32-run.exe` is not in PATH:

    set T32_RUN=O:\Foundry\t32-run\t32-run.exe
    make -C validation test

The assembler must support `.org 0x1000`.
