ASM ?= t32-as
PYTHON ?= python
NAME := subi
SOURCES := subi_normal.s subi_zero_imm.s subi_equal.s subi_borrow.s subi_overflow.s subi_alias.s
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
