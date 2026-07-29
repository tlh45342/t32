ASM ?= t32-as
PYTHON ?= python
NAME := mul
SOURCES := mul_normal.s mul_zero.s mul_identity.s mul_negative_positive.s mul_negative_negative.s mul_wrap_low32.s mul_alias.s
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
