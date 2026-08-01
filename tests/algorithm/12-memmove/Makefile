ASM ?= t32-as
PYTHON ?= python

SOURCE = memmove.s
BINARY = memmove.bin
LOG = memmove.log

all: $(BINARY)

$(BINARY): $(SOURCE)
	$(ASM) -f bin $(SOURCE) -o $(BINARY)

test: $(BINARY)
	$(PYTHON) run_tests.py

clean:
	-rm -f $(BINARY) $(LOG)

.PHONY: all test clean
