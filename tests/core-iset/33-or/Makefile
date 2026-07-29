ASM ?= t32-as
PYTHON ?= python
NAME := or
SOURCES := or_normal.s or_zero.s or_identity.s or_all_ones.s or_negative.s or_alias.s
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
