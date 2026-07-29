# 01-memory-copy

Validates a byte-oriented memory-copy algorithm using ordinary RAM.

The test initializes a 16-byte source buffer at:

```text
0x00009000

It clears a destination buffer at:

0x00009100

It then copies all 16 bytes from source to destination and verifies the
destination against the source.

Test data
00 11 22 33 44 55 66 77
88 99 AA BB CC DD EE FF
Result
r7 = 0  PASS
r7 = 1  FAIL
Instructions exercised together
MOVI
LDB
STB
ADDI
SUBI
XOR
JNZ
HALT

This test proves the basic algorithm later needed for:

memcpy;
executable loading;
disk-to-memory transfers;
console buffer operations;
packet-buffer movement;
section placement by loaders;
runtime initialization.

This version deliberately copies only non-overlapping buffers. Overlapping
memory movement belongs in a later memory-move test.


There is one important distinction already worth preserving:

```text
memory-copy

means non-overlapping buffers, like memcpy.

Later we should add:

06-memory-move

or similar, which detects overlap and selects forward or backward copying, like memmove.

But the next immediate progression should remain:

00-memory-fill
01-memory-copy
02-string-length
03-string-compare

After those four are running, we should pause before integer conversion and define the first small ABI document. That will let int-to-string, string-to-int, and future console routines become genuine callable functions rather than isolated test programs.