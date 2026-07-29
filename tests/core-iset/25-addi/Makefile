ASM ?= t32-as
PYTHON ?= python
NAME := addi
SOURCES := addi_normal.s addi_zero_imm.s addi_carry.s addi_overflow.s addi_negative.s addi_alias.s
BINARIES := $(SOURCES:.s=.bin)
LOGS := $(SOURCES:.s=.log)

all: $(BINARIES)

%.bin: %.s
	$(ASM) -f bin $< -o $@

test: $(BINARIES)
	$(PYTHON) run_tests.py

clean:
	-$(RM) $(BINARIES) $(LOGS)

.PHONY: all test clean
