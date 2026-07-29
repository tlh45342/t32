ASM ?= t32-as
PYTHON ?= python
SOURCES := ret_basic.s ret_nested.s ret_sequential.s ret_stack_data_survives.s ret_return_to_movi.s
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
