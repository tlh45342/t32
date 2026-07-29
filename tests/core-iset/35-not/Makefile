ASM ?= t32-as
PYTHON ?= python
NAME := not
SOURCES := not_normal.s not_zero.s not_ones.s not_alternating.s not_high_bit.s not_alias.s
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
