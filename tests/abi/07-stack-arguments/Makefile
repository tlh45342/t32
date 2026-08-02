ASM ?= t32-as
PYTHON ?= python
SOURCE = stack-arguments.s
BINARY = stack-arguments.bin
LOG = stack-arguments.log
all: $(BINARY)
$(BINARY): $(SOURCE)
	$(ASM) -f bin $(SOURCE) -o $(BINARY)
test: $(BINARY)
	$(PYTHON) run_tests.py
clean:
	-rm -f $(BINARY) $(LOG)
.PHONY: all test clean
