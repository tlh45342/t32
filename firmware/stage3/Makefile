T32_CC ?= t32-cc
T32_AS ?= t32-as
T32_LD ?= t32-ld

TARGET := next.bin
MAP := build/next.map
C_SOURCE := main.c
CRT_SOURCE := stage3.s
C_ASM := build/main.s
C_OBJECT := build/main.o
CRT_OBJECT := build/stage3.o
LIBT32 := ../../runtime/libt32/build/libt32.a

ifeq ($(OS),Windows_NT)
	PYTHON ?= python
	EXEEXT := .exe
	MKDIR_BUILD = if not exist build mkdir build
	RM_BUILD = if exist build rmdir /S /Q build
	RM_TARGET = if exist "$(TARGET)" del /F /Q "$(TARGET)"
else
	PYTHON ?= python3
	EXEEXT :=
	MKDIR_BUILD = mkdir -p build
	RM_BUILD = rm -rf build
	RM_TARGET = rm -f "$(TARGET)"
endif

T32_RUN ?= ../../vm/t32-run/bin/t32-run$(EXEEXT)

.PHONY: all test clean install libt32

all: $(TARGET)

build:
	@$(MKDIR_BUILD)

$(C_ASM): $(C_SOURCE) | build
	$(T32_CC) -S $(C_SOURCE) -o $(C_ASM)

$(C_OBJECT): $(C_ASM) | build
	$(T32_AS) -f obj $(C_ASM) -o $(C_OBJECT)

$(CRT_OBJECT): $(CRT_SOURCE) | build
	$(T32_AS) -f obj $(CRT_SOURCE) -o $(CRT_OBJECT)

libt32:
	$(MAKE) --no-print-directory -C ../../runtime/libt32 archive

$(TARGET): $(CRT_OBJECT) $(C_OBJECT) libt32
	$(T32_LD) -Ttext 0x00020000 $(CRT_OBJECT) $(C_OBJECT) $(LIBT32) -Map $(MAP) -o $(TARGET)

test: $(TARGET)
	$(PYTHON) tests/run_tests.py "$(T32_RUN)" "$(TARGET)" "$(MAP)"

install: $(TARGET)
ifeq ($(OS),Windows_NT)
	@if not exist "$(USERPROFILE)\.local\share\t32\boot" mkdir "$(USERPROFILE)\.local\share\t32\boot"
	copy /Y "$(TARGET)" "$(USERPROFILE)\.local\share\t32\boot\next.bin" >NUL
else
	mkdir -p "$(HOME)/.local/share/t32/boot"
	cp -f "$(TARGET)" "$(HOME)/.local/share/t32/boot/next.bin"
endif

clean:
	@$(RM_BUILD)
	@$(RM_TARGET)
