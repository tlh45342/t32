ASM ?= t32-as
PYTHON ?= python
SOURCES := jnz_taken_positive.s jnz_not_taken_zero.s jnz_taken_negative.s jnz_over_multiword.s jnz_source_preserved.s jnz_backward_loop.s
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
