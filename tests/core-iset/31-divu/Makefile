ASM ?= t32-as
PYTHON ?= python
NAME := divu
SOURCES := divu_normal.s divu_smaller_by_larger.s divu_max_by_two.s divu_high_bit_by_two.s divu_zero_dividend.s divu_identity.s divu_alias.s
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
