ASM ?= t32-as
PYTHON ?= python

SOURCES := reserved_05.s reserved_06.s reserved_07.s
BINARIES := $(SOURCES:.s=.bin)

all: $(BINARIES)

%.bin: %.s
	$(ASM) -f bin $< -o $@

test: all
	$(PYTHON) run_tests.py

clean:
	-rm -f $(BINARIES) *.log __reserved_mnemonic_*.s __reserved_mnemonic_*.bin

.PHONY: all test clean
