ASM ?= t32-as
PYTHON ?= python
NAME := div
SOURCES := div_normal.s div_positive_negative.s div_negative_positive.s div_negative_negative.s div_zero_dividend.s div_truncate_positive.s div_truncate_negative.s div_alias.s
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
