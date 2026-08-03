PYTHON := python

TOOLCHAIN_DIR := toolchain
VM_DIR        := vm
RUNTIME_DIR   := runtime
TESTS_DIR     := tests

T32_AS_DIR  := $(CURDIR)/$(TOOLCHAIN_DIR)/t32-as
T32_NM_DIR  := $(CURDIR)/$(TOOLCHAIN_DIR)/t32-nm
T32_LD_DIR  := $(CURDIR)/$(TOOLCHAIN_DIR)/t32-ld
T32_AR_DIR  := $(CURDIR)/$(TOOLCHAIN_DIR)/t32-ar
T32_CC_DIR  := $(CURDIR)/$(TOOLCHAIN_DIR)/t32-cc
T32_RUN_DIR := $(CURDIR)/$(VM_DIR)/t32-run/bin

# Prefer executables built in this checkout over older installed copies.
ifeq ($(OS),Windows_NT)
LOCAL_TOOL_PATH := $(T32_AS_DIR);$(T32_NM_DIR);$(T32_LD_DIR);$(T32_AR_DIR);$(T32_CC_DIR);$(T32_RUN_DIR)
RUN_LOCAL = set "PATH=$(LOCAL_TOOL_PATH);%PATH%" &&
else
LOCAL_TOOL_PATH := $(T32_AS_DIR):$(T32_NM_DIR):$(T32_LD_DIR):$(T32_AR_DIR):$(T32_CC_DIR):$(T32_RUN_DIR)
RUN_LOCAL = PATH="$(LOCAL_TOOL_PATH):$$PATH"
endif

all: tools vm runtime

# ----------------------------------------------------------------------
# Build
# ----------------------------------------------------------------------

tools:
	$(MAKE) -C $(TOOLCHAIN_DIR)/t32-as
	$(MAKE) -C $(TOOLCHAIN_DIR)/t32-nm
	$(MAKE) -C $(TOOLCHAIN_DIR)/t32-ld
	$(MAKE) -C $(TOOLCHAIN_DIR)/t32-ar
	$(MAKE) -C $(TOOLCHAIN_DIR)/t32-cc

vm:
	$(MAKE) -C $(VM_DIR)/t32-run

runtime: tools vm
	$(RUN_LOCAL) $(MAKE) -C $(RUNTIME_DIR)/libt32
	$(RUN_LOCAL) $(MAKE) -C $(RUNTIME_DIR)/crt0

# ----------------------------------------------------------------------
# Tests
# ----------------------------------------------------------------------

test-tools: tools
	$(MAKE) -C $(TOOLCHAIN_DIR)/t32-as test
	$(RUN_LOCAL) $(MAKE) -C $(TOOLCHAIN_DIR)/t32-nm test
	$(RUN_LOCAL) $(MAKE) -C $(TOOLCHAIN_DIR)/t32-ld test
	$(RUN_LOCAL) $(MAKE) -C $(TOOLCHAIN_DIR)/t32-ar test
	$(RUN_LOCAL) $(MAKE) -C $(TOOLCHAIN_DIR)/t32-cc test

test-vm: vm
	$(MAKE) -C $(VM_DIR)/t32-run test

test-abi: tools vm
	$(RUN_LOCAL) $(MAKE) -C $(TESTS_DIR)/abi test

test-runtime: tools vm
	$(RUN_LOCAL) $(MAKE) -C $(RUNTIME_DIR)/libt32 test
	$(RUN_LOCAL) $(MAKE) -C $(RUNTIME_DIR)/crt0 test

test: test-tools test-vm test-abi test-runtime

check: test

# ----------------------------------------------------------------------
# Installation
# ----------------------------------------------------------------------

install-tools: tools
	$(MAKE) -C $(TOOLCHAIN_DIR)/t32-as install
	$(MAKE) -C $(TOOLCHAIN_DIR)/t32-nm install
	$(MAKE) -C $(TOOLCHAIN_DIR)/t32-ld install
	$(MAKE) -C $(TOOLCHAIN_DIR)/t32-ar install
	$(MAKE) -C $(TOOLCHAIN_DIR)/t32-cc install

install-vm: vm
	$(MAKE) -C $(VM_DIR)/t32-run install

install-runtime: runtime
	$(RUN_LOCAL) $(MAKE) -C $(RUNTIME_DIR)/libt32 install
	$(RUN_LOCAL) $(MAKE) -C $(RUNTIME_DIR)/crt0 install

install: install-tools install-vm install-runtime

# ----------------------------------------------------------------------
# Cleanup
# ----------------------------------------------------------------------

clean:
	$(MAKE) -C $(TOOLCHAIN_DIR)/t32-as clean
	$(MAKE) -C $(TOOLCHAIN_DIR)/t32-nm clean
	$(MAKE) -C $(TOOLCHAIN_DIR)/t32-ld clean
	$(MAKE) -C $(TOOLCHAIN_DIR)/t32-ar clean
	$(MAKE) -C $(TOOLCHAIN_DIR)/t32-cc clean
	$(MAKE) -C $(VM_DIR)/t32-run clean
	$(MAKE) -C $(RUNTIME_DIR)/libt32 clean
	$(MAKE) -C $(RUNTIME_DIR)/crt0 clean
	$(MAKE) -C $(TESTS_DIR)/abi clean

# ----------------------------------------------------------------------
# Help
# ----------------------------------------------------------------------

help:
	@echo T32 monorepo targets:
	@echo.
	@echo   make              Build toolchain, VM, and runtime
	@echo   make test         Run tool, VM, ABI, and runtime tests
	@echo   make check        Alias for make test
	@echo   make install      Install host tools, VM, and T32 runtime
	@echo   make clean        Remove generated build products
	@echo.
	@echo Component targets:
	@echo   make tools
	@echo   make vm
	@echo   make runtime
	@echo   make test-tools
	@echo   make test-vm
	@echo   make test-abi
	@echo   make test-runtime
	@echo   make install-tools
	@echo   make install-vm
	@echo   make install-runtime

.PHONY: all tools vm runtime \
        test check test-tools test-vm test-abi test-runtime \
        install install-tools install-vm install-runtime \
        clean help
