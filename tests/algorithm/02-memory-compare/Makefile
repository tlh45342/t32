ASM    ?= t32-as
RUNNER ?= t32-run
PYTHON ?= python

SOURCE = memory-compare.s
BINARY = memory-compare.bin
LOG    = memory-compare.log

all: $(BINARY)

$(BINARY): $(SOURCE)
	$(ASM) -f bin $(SOURCE) -o $(BINARY)

test: $(BINARY)
	$(PYTHON) run_tests.py

clean:
	-rm -f $(BINARY) $(LOG)

.PHONY: all test clean
