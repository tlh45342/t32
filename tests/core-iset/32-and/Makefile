ASM ?= t32-as
PYTHON ?= python
NAME := and
SOURCES := and_normal.s and_zero.s and_identity.s and_mask.s and_negative.s and_alias.s
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
