ASM ?= t32-as
PYTHON ?= python
NAME := helloworld
SOURCE := $(NAME).s
BINARY := $(NAME).bin
LOG := $(NAME).log

all: $(BINARY)

$(BINARY): $(SOURCE)
	$(ASM) -f bin $(SOURCE) -o $(BINARY)

test: $(BINARY)
	t32-run < test.script

clean:
	-$(RM) $(BINARY) $(LOG)

.PHONY: all test clean
