# First Linked T32 Program

## 1. Goal

Prove the complete separate-compilation path:

```text
main.s
strlen.s
   ↓
t32-as -f obj
   ↓
main.o
strlen.o
   ↓
t32-nm
   ↓
t32-ld
   ↓
program.bin
   ↓
t32-run
```

Expected final result:

```text
r1 = 5
```

## 2. main.s

```asm
.global _start
.extern strlen

.section .text

_start:
    movi r15, 0x0000F000
    movi r0, message
    call strlen
    halt

.section .data

message:
    .byte 'H', 'e', 'l', 'l', 'o', 0
```

## 3. strlen.s

```asm
.global strlen

.section .text

strlen:
    movi r1, 0

strlen_loop:
    ldb  r2, [r0]
    jz   r2, strlen_done
    addi r0, r0, 1
    addi r1, r1, 1
    jmp  strlen_loop

strlen_done:
    ret
```

## 4. Assemble

```text
t32-as -f obj main.s -o main.o
t32-as -f obj strlen.s -o strlen.o
```

## 5. Inspect symbols

```text
t32-nm main.o
```

Expected conceptual output:

```text
00000000 T _start
         U strlen
00000000 d message
```

```text
t32-nm strlen.o
```

Expected conceptual output:

```text
00000000 T strlen
00000008 t strlen_loop
00000024 t strlen_done
```

Exact offsets depend on the final instruction encoding and section contents.

## 6. Link

```text
t32-ld -Ttext 0x00001000 main.o strlen.o \
    -Map program.map \
    -o program.bin
```

## 7. Run

```text
t32-run
load program.bin 0x00001000
set pc 0x00001000
run
regs
```

Expected:

```text
r1 = 0x00000005
state=halted
reason=HALT instruction
```

## 8. What this proves

- `.global` emission;
- `.extern` emission;
- `.text` and `.data`;
- address relocation for `message`;
- control-flow relocation for `strlen`;
- global symbol resolution;
- section placement;
- final binary execution.
