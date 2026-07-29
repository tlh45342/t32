ASSEMBLER ?= t32-as
PYTHON ?= python
BINARIES := stw_roundtrip.bin stw_little_endian_bytes.bin stw_overwrite.bin stw_unaligned.bin stw_zero.bin stw_flags_preserved.bin

all: $(BINARIES)

%.bin: %.s
	$(ASSEMBLER) -f bin $< -o $@

test: all
	$(PYTHON) run_tests.py

clean:
	$(RM) $(BINARIES) stw_*.log

.PHONY: all test clean
