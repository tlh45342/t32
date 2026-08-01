ASM ?= t32-as
PYTHON ?= python

SOURCE = hex-to-string.s
BINARY = hex-to-string.bin
LOG = hex-to-string.log

all: $(BINARY)

$(BINARY): $(SOURCE)
	$(ASM) -f bin $(SOURCE) -o $(BINARY)

test: $(BINARY)
	$(PYTHON) run_tests.py

clean:
	-rm -f $(BINARY) $(LOG)

.PHONY: all test clean
