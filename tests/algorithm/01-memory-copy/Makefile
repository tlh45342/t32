ASM ?= t32-as
PYTHON ?= python

all: memory-copy.bin

memory-copy.bin: memory-copy.s
	$(ASM) -f bin memory-copy.s -o memory-copy.bin

test: memory-copy.bin
	$(PYTHON) run_tests.py

clean:
	rm -f memory-copy.bin memory-copy.log

.PHONY: all test clean