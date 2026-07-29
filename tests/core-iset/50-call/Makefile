ASM ?= t32-as
PYTHON ?= python
SOURCES := call_forward.s call_nested.s call_sequential.s call_return_after_multiword.s call_existing_stack_data.s call_preserve_flags.s
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
