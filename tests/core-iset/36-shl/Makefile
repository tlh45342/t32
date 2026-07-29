ASM ?= t32-as
PYTHON ?= python
NAME := shl
SOURCES := shl_normal.s shl_zero_count.s shl_by_31.s shl_carry_out.s shl_zero_result.s shl_negative_result.s shl_alias.s
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
