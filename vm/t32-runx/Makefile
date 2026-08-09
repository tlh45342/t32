# t32-runx 0.0.6
#
# Windows-only interactive developer host for T32.
# Canonical VM implementation is owned by ../libt32vm.

CC      := gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -Wpedantic -O2
PYTHON  ?= python

COPY_FILE = copy /Y "$(subst /,\,$1)" "$(subst /,\,$2)" >NUL

LIBT32VM_DIR := ../libt32vm
T32_INCLUDE  := $(LIBT32VM_DIR)/include
T32_LIB      := $(LIBT32VM_DIR)/lib/libt32vm.a

BUILD_DIR := build
BIN_DIR   := bin
TARGET    := $(BIN_DIR)/t32-runx.exe
OBJECT    := $(BUILD_DIR)/runx.o

.PHONY: all clean test install

all: $(TARGET)

$(T32_LIB):
	$(MAKE) -C $(LIBT32VM_DIR)

$(BUILD_DIR):
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"

$(BIN_DIR):
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"

$(OBJECT): src/runx.c | $(BUILD_DIR)
	$(CC) -I"$(T32_INCLUDE)" $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECT) $(T32_LIB) | $(BIN_DIR)
	$(CC) $(OBJECT) "$(T32_LIB)" -mwindows -lgdi32 -luser32 -lcomdlg32 -o $@

test: all
	$(PYTHON) tests/run_tests.py

install: all
	@if not exist "$(USERPROFILE)\.local\bin" mkdir "$(USERPROFILE)\.local\bin"
	@$(call COPY_FILE,$(TARGET),$(USERPROFILE)/.local/bin/t32-runx.exe)
	@echo Installed t32-runx.exe to $(USERPROFILE)\.local\bin

clean:
	@if exist "$(BUILD_DIR)" rmdir /S /Q "$(BUILD_DIR)"
	@if exist "$(BIN_DIR)" rmdir /S /Q "$(BIN_DIR)"
