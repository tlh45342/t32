ASM ?= t32-as
PYTHON ?= python
NAME := sub
SOURCES := sub_normal.s sub_equal.s sub_borrow.s sub_underflow.s sub_overflow.s sub_alias.s
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
