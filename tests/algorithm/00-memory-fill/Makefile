ASM := t32-as
PYTHON := python

SOURCE := memory-fill.s
BINARY := memory-fill.bin
LOG := memory-fill.log

all: $(BINARY)

$(BINARY): $(SOURCE)
	$(ASM) -f bin $(SOURCE) -o $(BINARY)

test: $(BINARY)
	$(PYTHON) run_tests.py

clean:
	rm -f $(BINARY) $(LOG)

.PHONY: all test clean
