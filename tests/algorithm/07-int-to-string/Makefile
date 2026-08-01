ASM ?= t32-as
RUN ?= t32-run
PYTHON ?= python

PROGRAM := int-to-string
SOURCE  := $(PROGRAM).s
BINARY  := $(PROGRAM).bin
LOG     := $(PROGRAM).log

all: $(BINARY)

$(BINARY): $(SOURCE)
	$(ASM) -f bin $(SOURCE) -o $(BINARY)

test: $(BINARY)
	$(PYTHON) run_tests.py

clean:
	rm -f $(BINARY) $(LOG)

.PHONY: all test clean
