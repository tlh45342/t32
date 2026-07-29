ASSEMBLER ?= t32-as
PYTHON ?= python
BINARIES := sth_low_half_only.bin sth_little_endian_bytes.bin sth_upper_half_preserved.bin sth_unaligned.bin sth_overwrite.bin sth_flags_preserved.bin

all: $(BINARIES)

%.bin: %.s
	$(ASSEMBLER) -f bin $< -o $@

test: all
	$(PYTHON) run_tests.py

clean:
	$(RM) $(BINARIES) sth_*.log

.PHONY: all test clean
