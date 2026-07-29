ASSEMBLER ?= t32-as
PYTHON ?= python
BINARIES := ldw_roundtrip.bin ldw_little_endian_bytes.bin ldw_unaligned.bin ldw_overwrite.bin ldw_zero.bin ldw_flags_preserved.bin

all: $(BINARIES)

%.bin: %.s
	$(ASSEMBLER) -f bin $< -o $@

test: all
	$(PYTHON) run_tests.py

clean:
	$(RM) $(BINARIES) ldw_*.log

.PHONY: all test clean
