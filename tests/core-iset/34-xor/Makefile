ASM ?= t32-as
PYTHON ?= python
NAME := xor
SOURCES := xor_normal.s xor_identity.s xor_self_zero.s xor_alternating.s xor_negative.s xor_alias.s
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
