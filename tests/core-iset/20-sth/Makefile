ASM ?= t32-asm
PYTHON ?= python
NAME := sth
SOURCE := $(NAME).s
BINARY := $(NAME).bin
LOG := $(NAME).log

all: $(BINARY)

$(BINARY): $(SOURCE)
	$(ASM) -f bin $(SOURCE) -o $(BINARY)

test: $(BINARY)
	$(PYTHON) run_tests.py

clean:
	-$(RM) $(BINARY) $(LOG)

.PHONY: all test clean
