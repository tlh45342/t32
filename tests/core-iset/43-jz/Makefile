ASM ?= t32-as
PYTHON ?= python
SOURCES := jz_taken_zero.s jz_not_taken_positive.s jz_not_taken_negative.s jz_over_multiword.s jz_source_preserved.s jz_backward_exit.s
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
