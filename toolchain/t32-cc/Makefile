# t32-cc Makefile
#
# Builds src/t32-cc.c and installs to the current user's local profile:
#   Windows: %USERPROFILE%/.local/bin
#   Linux:   $HOME/.local/bin
#   macOS:   $HOME/.local/bin

PROJECT   := t32-cc
VERSION   ?= 0.0.3
SRC_DIR   := src
BUILD_DIR := build
TEST_DIR  := tests
PYTHON    ?= python

ifeq ($(OS),Windows_NT)
    CC := gcc
    EXEEXT := .exe

    PREFIX ?= $(USERPROFILE)/.local
    BINDIR ?= $(PREFIX)/bin

    MKDIR_P = if not exist "$(subst /,\,$1)" mkdir "$(subst /,\,$1)"
    RM_RF = if exist "$(subst /,\,$1)" rmdir /S /Q "$(subst /,\,$1)"
    RM_FILE = if exist "$(subst /,\,$1)" del /F /Q "$(subst /,\,$1)"
    COPY_FILE = copy /Y "$(subst /,\,$1)" "$(subst /,\,$2)" >NUL
else
    CC ?= cc
    EXEEXT :=

    PREFIX ?= $(HOME)/.local
    BINDIR ?= $(PREFIX)/bin

    MKDIR_P = mkdir -p "$1"
    RM_RF = rm -rf "$1"
    RM_FILE = rm -f "$1"
    COPY_FILE = cp -f "$1" "$2"
endif

CPPFLAGS ?=
CFLAGS   ?= -std=c11 -Wall -Wextra -Wpedantic -O2
LDFLAGS  ?=
LDLIBS   ?=

TARGET := $(PROJECT)$(EXEEXT)
SOURCE := $(SRC_DIR)/t32-cc.c
OBJECT := $(BUILD_DIR)/t32-cc.o

all: $(TARGET)

$(TARGET): $(OBJECT)
	$(CC) $(LDFLAGS) $(OBJECT) $(LDLIBS) -o $@

$(OBJECT): $(SOURCE)
	@$(call MKDIR_P,$(BUILD_DIR))
	$(CC) $(CPPFLAGS) $(CFLAGS) \
		-DT32CC_VERSION=\"$(VERSION)\" \
		-c $< -o $@

test: $(TARGET)
	$(PYTHON) $(TEST_DIR)/run_tests.py ./$(TARGET)

install: $(TARGET)
	@$(call MKDIR_P,$(BINDIR))
	@$(call COPY_FILE,$(TARGET),$(BINDIR)/$(TARGET))
	@echo Installed $(TARGET) to $(BINDIR)

uninstall:
	@$(call RM_FILE,$(BINDIR)/$(TARGET))
	@echo Removed $(BINDIR)/$(TARGET)

clean:
	@$(call RM_RF,$(BUILD_DIR))
	@$(call RM_RF,$(TEST_DIR)/build)
	@$(call RM_FILE,$(TARGET))

rebuild: clean all

show-config:
	@echo Project:     $(PROJECT)
	@echo Version:     $(VERSION)
	@echo Compiler:    $(CC)
	@echo Source:      $(SOURCE)
	@echo Object:      $(OBJECT)
	@echo Target:      $(TARGET)
	@echo Prefix:      $(PREFIX)
	@echo Binary dir:  $(BINDIR)
	@echo Python:      $(PYTHON)

.PHONY: all test install uninstall clean rebuild show-config
