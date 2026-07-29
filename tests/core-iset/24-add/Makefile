ASM ?= t32-as
PYTHON ?= python
NAME := add
SOURCES := add_normal.s add_zero.s add_carry.s add_overflow.s add_negative.s add_alias.s
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
