ASM := t32-as
PYTHON := python

SOURCE := string-length.s
BINARY := string-length.bin
LOG := string-length.log

all: $(BINARY)

$(BINARY): $(SOURCE)
	$(ASM) -f bin $(SOURCE) -o $(BINARY)

test: $(BINARY)
	$(PYTHON) run_tests.py

clean:
	rm -f $(BINARY) $(LOG)

.PHONY: all test clean
