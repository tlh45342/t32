ASM ?= t32-as
PYTHON ?= python
NAME := cmpi
SOURCES := cmpi_equal.s cmpi_greater.s cmpi_less.s cmpi_signed_overflow.s cmpi_zero.s
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
