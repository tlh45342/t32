ASSEMBLER ?= t32-as
PYTHON ?= python
BINARIES := ldb_zero_extend_ff.bin ldb_offset_bytes.bin ldb_overwrite.bin ldb_source_preserved.bin ldb_flags_preserved.bin ldb_unaligned_from_word.bin

all: $(BINARIES)

%.bin: %.s
	$(ASSEMBLER) -f bin $< -o $@

test: all
	$(PYTHON) run_tests.py

clean:
	$(RM) $(BINARIES) ldb_*.log

.PHONY: all test clean
