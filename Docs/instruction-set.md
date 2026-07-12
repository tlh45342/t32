# T32 Instruction Set

## HALT

Syntax

    halt

Description

Stops execution.

Flags

None.

Encoding

Opcode 0x00

Example

    halt

---------------------------------------

## MOVI

Syntax

    movi rN, immediate

Description

Loads a 32-bit immediate.

Flags

Z
N

Encoding

Opcode 0x02

Example

    movi r0,47
