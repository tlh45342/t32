ASM ?= t32-as
PYTHON ?= python

SOURCE = strchr.s
BINARY = strchr.bin
LOG = strchr.log

all: $(BINARY)

$(BINARY): $(SOURCE)
	$(ASM) -f bin $(SOURCE) -o $(BINARY)

test: $(BINARY)
	$(PYTHON) run_tests.py

clean:
	-rm -f $(BINARY) $(LOG)

.PHONY: all test clean
