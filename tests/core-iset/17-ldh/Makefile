ASSEMBLER ?= t32-as
PYTHON ?= python
BINARIES := ldh_zero_extend_ffff.bin ldh_little_endian_low.bin ldh_little_endian_high.bin ldh_unaligned.bin ldh_overwrite.bin ldh_flags_preserved.bin

all: $(BINARIES)

%.bin: %.s
	$(ASSEMBLER) -f bin $< -o $@

test: all
	$(PYTHON) run_tests.py

clean:
	$(RM) $(BINARIES) ldh_*.log

.PHONY: all test clean
