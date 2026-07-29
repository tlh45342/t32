ASM ?= t32-as
PYTHON ?= python
NAME := sar
SOURCES := sar_negative.s sar_positive.s sar_zero_count.s sar_by_31_negative.s sar_by_31_positive.s sar_carry_out.s sar_alias.s
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
