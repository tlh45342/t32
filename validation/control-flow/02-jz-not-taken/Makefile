ASM ?= t32-asm
PYTHON ?= python
NAME := jz-not-taken

all: $(NAME).bin

$(NAME).bin: $(NAME).s
	$(ASM) -f bin $(NAME).s -o $(NAME).bin

test: $(NAME).bin
	$(PYTHON) run_tests.py

clean:
ifeq ($(OS),Windows_NT)
	-del /Q $(NAME).bin $(NAME).log 2>NUL
else
	rm -f $(NAME).bin $(NAME).log
endif

.PHONY: all test clean
