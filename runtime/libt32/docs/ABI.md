# libt32 ABI conformance

`libt32 0.0.4` conforms to the draft T32 ABI 0.1.

- Arguments use `r0-r3`.
- Primary results use `r0`.
- Optional secondary status uses `r1`.
- Public routines preserve `r8-r14` and restore `r15`.
- `r0-r7` are caller-saved.

The machine-wide normative specification lives in `t32/docs/abi/`.
