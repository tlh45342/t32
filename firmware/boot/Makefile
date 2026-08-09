T32_AS ?= t32-as
TARGET := boot.bin
SOURCE := boot.s

ifeq ($(OS),Windows_NT)
	EXEEXT := .exe
	PYTHON ?= python
else
	EXEEXT :=
	PYTHON ?= python3
endif

T32_RUN ?= ../../vm/t32-run/bin/t32-run$(EXEEXT)

.PHONY: all test clean install

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(T32_AS) -f bin $(SOURCE) -o $(TARGET)

test: $(TARGET)
	$(PYTHON) tests/run_tests.py "$(T32_RUN)" "$(TARGET)"

install: $(TARGET)
ifeq ($(OS),Windows_NT)
	@if not exist "$(USERPROFILE)\.local\share\t32\boot" mkdir "$(USERPROFILE)\.local\share\t32\boot"
	copy /Y "$(TARGET)" "$(USERPROFILE)\.local\share\t32\boot\boot.bin" >NUL
else
	mkdir -p "$(HOME)/.local/share/t32/boot"
	cp -f "$(TARGET)" "$(HOME)/.local/share/t32/boot/boot.bin"
endif

clean:
ifeq ($(OS),Windows_NT)
	@if exist "$(TARGET)" del /Q "$(TARGET)"
else
	rm -f "$(TARGET)"
endif
