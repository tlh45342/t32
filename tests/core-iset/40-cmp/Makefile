ASM ?= t32-as
PYTHON ?= python
NAME := cmp
SOURCES := cmp_equal.s cmp_greater.s cmp_less.s cmp_signed_overflow.s cmp_negative_equal.s
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
