ASM ?= t32-as
PYTHON ?= python
SOURCES := push_single.s push_roundtrip.s push_lifo.s push_zero.s push_high_bit.s push_four_values.s
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
