ASM ?= t32-as
PYTHON ?= python
SOURCES := pop_single.s pop_overwrite.s pop_lifo.s pop_zero.s pop_negative.s pop_source_preserved.s
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
