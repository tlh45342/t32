# t32-disk 0.0.1

ifeq ($(OS),Windows_NT)
	CC := gcc
	EXEEXT := .exe
	PREFIX ?= $(USERPROFILE)/.local
	BINDIR ?= $(PREFIX)/bin
	MKDIR_P = if not exist "$(subst /,\,$1)" mkdir "$(subst /,\,$1)"
	RM_RF = if exist "$(subst /,\,$1)" rmdir /S /Q "$(subst /,\,$1)"
	COPY_FILE = copy /Y "$(subst /,\,$1)" "$(subst /,\,$2)" >NUL
else
	CC ?= cc
	EXEEXT :=
	PREFIX ?= $(HOME)/.local
	BINDIR ?= $(PREFIX)/bin
	MKDIR_P = mkdir -p "$1"
	RM_RF = rm -rf "$1"
	COPY_FILE = cp -f "$1" "$2"
endif

CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
PYTHON ?= python

BUILD_DIR := build
BIN_DIR := bin
TARGET := $(BIN_DIR)/t32-disk$(EXEEXT)
OBJECT := $(BUILD_DIR)/t32-disk.o

.PHONY: all test install clean

all: $(TARGET)

$(BUILD_DIR):
	@$(call MKDIR_P,$(BUILD_DIR))

$(BIN_DIR):
	@$(call MKDIR_P,$(BIN_DIR))

$(OBJECT): src/t32-disk.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECT) | $(BIN_DIR)
	$(CC) $(OBJECT) -o $@

test: $(TARGET)
	$(PYTHON) tests/run_tests.py "$(TARGET)"

install: $(TARGET)
	@$(call MKDIR_P,$(BINDIR))
	@$(call COPY_FILE,$(TARGET),$(BINDIR)/t32-disk$(EXEEXT))
	@echo Installed t32-disk$(EXEEXT) to $(BINDIR)

clean:
	@$(call RM_RF,$(BUILD_DIR))
	@$(call RM_RF,$(BIN_DIR))
