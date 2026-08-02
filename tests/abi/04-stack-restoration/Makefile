ASM ?= t32-as
PYTHON ?= python
SOURCE = stack-restoration.s
BINARY = stack-restoration.bin
LOG = stack-restoration.log
all: $(BINARY)
$(BINARY): $(SOURCE)
	$(ASM) -f bin $(SOURCE) -o $(BINARY)
test: $(BINARY)
	$(PYTHON) run_tests.py
clean:
	-rm -f $(BINARY) $(LOG)
.PHONY: all test clean
