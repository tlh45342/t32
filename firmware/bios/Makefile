T32_AS ?= t32-as
TARGET := bios.bin
SOURCE := bios.s

ifeq ($(OS),Windows_NT)
	EXEEXT := .exe
	PYTHON ?= python
	RM_FILE = if exist "$(subst /,\,$1)" del /Q "$(subst /,\,$1)"
else
	EXEEXT :=
	PYTHON ?= python3
	RM_FILE = rm -f "$1"
endif

RUNNER ?= ../../vm/t32-run/bin/t32-run$(EXEEXT)
DISK_TOOL ?= ../../tools/t32-disk/bin/t32-disk$(EXEEXT)
BOOT_IMAGE := ../boot/boot.bin
NEXT_IMAGE := ../stage3/next.bin
SUBMAKE := $(MAKE) --no-print-directory

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(T32_AS) -f bin $(SOURCE) -o $(TARGET)

test: $(TARGET)
	$(SUBMAKE) -C ../boot
	$(SUBMAKE) -C ../stage3
	$(SUBMAKE) -C ../../tools/t32-disk
	$(SUBMAKE) -C ../../vm/t32-run
	$(PYTHON) tests/run_tests.py "$(RUNNER)" "$(TARGET)" "$(BOOT_IMAGE)" "$(NEXT_IMAGE)" "$(DISK_TOOL)"

clean:
	@$(call RM_FILE,$(TARGET))
