ASM ?= t32-as
PYTHON ?= python
SOURCE = stack-alignment.s
BINARY = stack-alignment.bin
LOG = stack-alignment.log
all: $(BINARY)
$(BINARY): $(SOURCE)
	$(ASM) -f bin $(SOURCE) -o $(BINARY)
test: $(BINARY)
	$(PYTHON) run_tests.py
clean:
	-rm -f $(BINARY) $(LOG)
.PHONY: all test clean
