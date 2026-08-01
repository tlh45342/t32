ASM ?= t32-as
PYTHON ?= python

SOURCE = memchr.s
BINARY = memchr.bin
LOG = memchr.log

all: $(BINARY)

$(BINARY): $(SOURCE)
	$(ASM) -f bin $(SOURCE) -o $(BINARY)

test: $(BINARY)
	$(PYTHON) run_tests.py

clean:
	-rm -f $(BINARY) $(LOG)

.PHONY: all test clean
