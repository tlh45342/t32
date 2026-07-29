ASSEMBLER ?= t32-as
PYTHON ?= python
BINARIES := stb_low_byte_only.bin stb_neighbor_preserved.bin stb_overwrite.bin stb_build_word.bin stb_source_preserved.bin stb_flags_preserved.bin

all: $(BINARIES)

%.bin: %.s
	$(ASSEMBLER) -f bin $< -o $@

test: all
	$(PYTHON) run_tests.py

clean:
	$(RM) $(BINARIES) stb_*.log

.PHONY: all test clean
