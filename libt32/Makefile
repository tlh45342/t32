PYTHON ?= python
ASM ?= t32-as
AR ?= t32-ar

all: check

check:
	$(PYTHON) tools/check_manifest.py

objects:
	@echo "libt32 objects require: t32-as -f obj"
	@echo "T32OBJ v1 has not been implemented yet."
	@exit 1

archive:
	@echo "libt32.a requires t32-as -f obj and t32-ar."
	@exit 1

clean:
	-rm -rf build/*
	-mkdir -p build

.PHONY: all check objects archive clean