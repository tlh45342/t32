ASM ?= t32-as
PYTHON ?= python
NAME := shr
SOURCES := shr_normal.s shr_zero_count.s shr_by_31.s shr_carry_out.s shr_logical_fill.s shr_zero_result.s shr_alias.s
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
