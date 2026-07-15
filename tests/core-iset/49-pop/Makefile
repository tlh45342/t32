AS :== t32-asm
ASMFLAGS ?= -f bin

EMU ?= t32-run.exe

ASM := pop.s
BIN := pop.bin

all: $(BIN)

$(BIN): $(ASM)
	$(AS) $(ASMFLAGS) $< -o $@

test: $(BIN)
	python run_tests.py

clean:
	-del /q $(BIN) 2>nul || exit 0

.PHONY: all test clean