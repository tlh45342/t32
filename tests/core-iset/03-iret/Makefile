ASM ?= t32-as
PYTHON ?= python
SOURCES := iret_forward.s iret_next_instruction.s iret_over_multiword.s iret_stack_data_below_pc.s iret_preserve_registers.s
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
