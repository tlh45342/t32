ASM ?= t32-as
PYTHON ?= python
NAME := mulu
SOURCES := mulu_normal.s mulu_zero.s mulu_identity.s mulu_max_times_two.s mulu_high_bit_wrap.s mulu_alternating.s mulu_alias.s
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
