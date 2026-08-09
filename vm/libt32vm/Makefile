# libt32vm
#
# Canonical host-native T32 virtual-machine core.
#
# Builds:
#   lib/libt32vm.a
#
# This project owns the T32 CPU, RAM, display MMIO, disk MMIO,
# keyboard MMIO, and platform-control MMIO implementation.

PROJECT := libt32vm

SRC_DIR   := src
INC_DIR   := include
BUILD_DIR := build
LIB_DIR   := lib
TEST_DIR  := tests

ifeq ($(OS),Windows_NT)
	CC := gcc
	AR := ar
	EXEEXT := .exe

	PREFIX ?= $(USERPROFILE)/.local
	LIBDIR ?= $(PREFIX)/lib
	INCLUDEDIR ?= $(PREFIX)/include

	MKDIR_P = if not exist "$(subst /,\,$1)" mkdir "$(subst /,\,$1)"
	RM_RF = if exist "$(subst /,\,$1)" rmdir /S /Q "$(subst /,\,$1)"
	COPY_FILE = copy /Y "$(subst /,\,$1)" "$(subst /,\,$2)" >NUL
else
	CC ?= cc
	AR ?= ar
	EXEEXT :=

	PREFIX ?= $(HOME)/.local
	LIBDIR ?= $(PREFIX)/lib
	INCLUDEDIR ?= $(PREFIX)/include

	MKDIR_P = mkdir -p "$1"
	RM_RF = rm -rf "$1"
	COPY_FILE = cp -f "$1" "$2"
endif

CPPFLAGS ?= -I$(INC_DIR)
ifneq ($(OS),Windows_NT)
	CPPFLAGS += -D_POSIX_C_SOURCE=200809L
endif

CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2

LIBT32VM := $(LIB_DIR)/libt32vm.a
CORE_OBJECT := $(BUILD_DIR)/t32.o
TEST_BIN := $(BUILD_DIR)/test_libt32vm$(EXEEXT)

.PHONY: all test install clean rebuild

all: $(LIBT32VM)

$(LIBT32VM): $(CORE_OBJECT)
	@$(call MKDIR_P,$(LIB_DIR))
	$(AR) rcs $@ $^

$(CORE_OBJECT): $(SRC_DIR)/t32.c $(INC_DIR)/t32.h $(INC_DIR)/t32_opcodes.h
	@$(call MKDIR_P,$(BUILD_DIR))
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(TEST_BIN): $(TEST_DIR)/test_libt32vm.c $(LIBT32VM)
	@$(call MKDIR_P,$(BUILD_DIR))
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBT32VM) -o $@

test: $(TEST_BIN)
	"$(TEST_BIN)"

install: all
	@$(call MKDIR_P,$(LIBDIR))
	@$(call MKDIR_P,$(INCLUDEDIR))
	@$(call COPY_FILE,$(LIBT32VM),$(LIBDIR)/libt32vm.a)
	@$(call COPY_FILE,$(INC_DIR)/t32.h,$(INCLUDEDIR)/t32.h)
	@$(call COPY_FILE,$(INC_DIR)/t32_opcodes.h,$(INCLUDEDIR)/t32_opcodes.h)
	@echo Installed libt32vm.a to $(LIBDIR)
	@echo Installed libt32vm headers to $(INCLUDEDIR)

clean:
	@$(call RM_RF,$(BUILD_DIR))
	@$(call RM_RF,$(LIB_DIR))

rebuild: clean all
