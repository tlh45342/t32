ASM ?= t32-as
PYTHON ?= python

SOURCE = string-to-hex.s
BINARY = string-to-hex.bin
LOG = string-to-hex.log

all: $(BINARY)

$(BINARY): $(SOURCE)
	$(ASM) -f bin $(SOURCE) -o $(BINARY)

test: $(BINARY)
	$(PYTHON) run_tests.py

clean:
	-rm -f $(BINARY) $(LOG)

.PHONY: all test clean
