ASM ?= t32-as
PYTHON ?= python
SOURCES := jmp_forward.s jmp_next_instruction.s jmp_over_multiword.s jmp_backward.s jmp_preserve_flags.s
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
