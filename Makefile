# T32 repository root Makefile
#
# Repository-wide build and validation orchestration.
#
# Component build logic remains in each component's own Makefile.
# The tests/ hierarchy is discovered by Makefile presence so new
# conformance cases join their established test group automatically.
#
# Primary entry points:
#
#   make do
#   make test
#   make test-all
#   make test-core
#   make test-architecture
#   make test-abi
#   make test-algorithm
#   make test-vm
#   make test-firmware
#   make list-tests
#
# t32-runx is buildable here, but intentionally NOT part of the
# mandatory test graph until its tests are confirmed headless/CLI-safe.

MAKE ?= make

# ----------------------------------------------------------------------
# Components
# ----------------------------------------------------------------------

TOOLCHAIN_DIRS := \
	toolchain/t32-as \
	toolchain/t32-ar \
	toolchain/t32-ld \
	toolchain/t32-nm \
	toolchain/t32-cc

VM_TEST_DIRS := \
	vm/libt32vm \
	vm/t32-run

RUNTIME_DIRS := \
	runtime/crt0 \
	runtime/libt32

FIRMWARE_DIRS := \
	firmware/bios \
	firmware/boot \
	firmware/stage3

TOOLS_DIRS := \
	tools/t32-disk

# ----------------------------------------------------------------------
# Repository validation discovery
# ----------------------------------------------------------------------

CORE_TEST_DIRS := $(sort \
	$(patsubst %/Makefile,%,$(wildcard tests/core-iset/*/Makefile)))

ARCH_TEST_DIRS := $(sort \
	$(patsubst %/Makefile,%,$(wildcard tests/architecture/*/Makefile)) \
	$(patsubst %/Makefile,%,$(wildcard tests/architecture/*/*/Makefile)))

ABI_TEST_DIRS := $(sort \
	$(patsubst %/Makefile,%,$(wildcard tests/abi/*/Makefile)))

ALGORITHM_TEST_DIRS := $(sort \
	$(patsubst %/Makefile,%,$(wildcard tests/algorithm/*/Makefile)))

PLATFORM_TEST_DIRS := $(sort \
	$(patsubst %/Makefile,%,$(wildcard tests/platform/*/Makefile)) \
	$(patsubst %/Makefile,%,$(wildcard tests/platform/*/*/Makefile)))

SYSTEM_TEST_DIRS := $(sort \
	$(patsubst %/Makefile,%,$(wildcard tests/system/*/Makefile)) \
	$(patsubst %/Makefile,%,$(wildcard tests/system/*/*/Makefile)))

ALL_DISCOVERED_TEST_DIRS := $(sort \
	$(CORE_TEST_DIRS) \
	$(ARCH_TEST_DIRS) \
	$(ABI_TEST_DIRS) \
	$(ALGORITHM_TEST_DIRS) \
	$(PLATFORM_TEST_DIRS) \
	$(SYSTEM_TEST_DIRS))

# ----------------------------------------------------------------------
# Convenience / developer path
# ----------------------------------------------------------------------

all: help

# Rebuild and install the active compiler first, then execute the
# normal trusted regression suite.  do.bat calls this target.
do:
	$(MAKE) prepare-cc
	$(MAKE) test

prepare-cc:
	$(MAKE) -C toolchain/t32-cc clean
	$(MAKE) -C toolchain/t32-cc
	$(MAKE) -C toolchain/t32-cc install

# ----------------------------------------------------------------------
# Trusted regression suite
# ----------------------------------------------------------------------

test: \
	test-toolchain \
	test-vm \
	test-core \
	test-architecture \
	test-abi \
	test-algorithm \
	test-runtime \
	test-firmware
	@echo
	@echo ============================================================
	@echo T32 trusted regression suite: PASS
	@echo ============================================================

tests: test

# Broader suite.  Platform and validation remain opt-in from the normal
# compiler-development loop because some areas may still be evolving.
test-all: test test-tools test-platform validation
	@echo
	@echo ============================================================
	@echo T32 full regression suite: PASS
	@echo ============================================================

# ----------------------------------------------------------------------
# Toolchain
# ----------------------------------------------------------------------

test-toolchain:
	@echo
	@echo ============================================================
	@echo Testing T32 toolchain
	@echo ============================================================
	$(MAKE) -C toolchain/t32-as test
	$(MAKE) -C toolchain/t32-ar test
	$(MAKE) -C toolchain/t32-ld test
	$(MAKE) -C toolchain/t32-nm test
	$(MAKE) -C toolchain/t32-cc test

# ----------------------------------------------------------------------
# VM
# ----------------------------------------------------------------------

build-vm:
	$(MAKE) -C vm/libt32vm
	$(MAKE) -C vm/t32-run
	$(MAKE) build-runx

build-runx:
ifneq ($(wildcard vm/t32-runx/Makefile),)
	$(MAKE) -C vm/t32-runx
else
	@echo t32-runx: no Makefile found; build skipped.
endif

test-vm:
	@echo
	@echo ============================================================
	@echo Testing T32 VM core and CLI runner
	@echo ============================================================
	$(MAKE) -C vm/libt32vm test
	$(MAKE) -C vm/t32-run test
	@echo t32-runx tests intentionally excluded pending headless confirmation.

# ----------------------------------------------------------------------
# Repository conformance groups
# ----------------------------------------------------------------------

test-core: $(CORE_TEST_DIRS)
	@echo T32 core ISA tests: PASS

test-architecture: $(ARCH_TEST_DIRS)
	@echo T32 architecture tests: PASS

test-abi: $(ABI_TEST_DIRS)
	@echo T32 ABI tests: PASS

test-algorithm: $(ALGORITHM_TEST_DIRS)
	@echo T32 algorithm tests: PASS

test-platform: $(PLATFORM_TEST_DIRS) $(SYSTEM_TEST_DIRS)
	@echo T32 platform/system tests: PASS

$(ALL_DISCOVERED_TEST_DIRS):
	@echo
	@echo ============================================================
	@echo Running test: $@
	@echo ============================================================
	$(MAKE) -C "$@" test

# ----------------------------------------------------------------------
# Runtime / firmware / tools / higher-level validation
# ----------------------------------------------------------------------

test-runtime:
	@echo
	@echo ============================================================
	@echo Testing T32 runtime
	@echo ============================================================
	$(MAKE) -C runtime/crt0 test
	$(MAKE) -C runtime/libt32 test

test-firmware:
	@echo
	@echo ============================================================
	@echo Testing T32 firmware
	@echo ============================================================
	$(MAKE) -C firmware/bios test
	$(MAKE) -C firmware/boot test
	$(MAKE) -C firmware/stage3 test

test-tools:
	@echo
	@echo ============================================================
	@echo Testing T32 tools
	@echo ============================================================
	$(MAKE) -C tools/t32-disk test

validation:
	$(MAKE) -C validation test

clean-validation:
	$(MAKE) -C validation clean

# ----------------------------------------------------------------------
# Discovery / diagnostics
# ----------------------------------------------------------------------

list-tests:
	@echo Core ISA:
	@$(foreach dir,$(CORE_TEST_DIRS),echo "  $(dir)";)
	@echo.
	@echo Architecture:
	@$(foreach dir,$(ARCH_TEST_DIRS),echo "  $(dir)";)
	@echo.
	@echo ABI:
	@$(foreach dir,$(ABI_TEST_DIRS),echo "  $(dir)";)
	@echo.
	@echo Algorithms:
	@$(foreach dir,$(ALGORITHM_TEST_DIRS),echo "  $(dir)";)
	@echo.
	@echo Platform/system:
	@$(foreach dir,$(PLATFORM_TEST_DIRS) $(SYSTEM_TEST_DIRS),echo "  $(dir)";)

list-core-tests:
	@$(foreach dir,$(CORE_TEST_DIRS),echo "$(dir)";)

list-architecture-tests:
	@$(foreach dir,$(ARCH_TEST_DIRS),echo "$(dir)";)

# ----------------------------------------------------------------------
# Cleaning
# ----------------------------------------------------------------------

clean-tests:
	@$(foreach dir,$(ALL_DISCOVERED_TEST_DIRS),$(MAKE) -C "$(dir)" clean || exit 1;)

clean:
	$(MAKE) -C toolchain/t32-as clean
	$(MAKE) -C toolchain/t32-ar clean
	$(MAKE) -C toolchain/t32-ld clean
	$(MAKE) -C toolchain/t32-nm clean
	$(MAKE) -C toolchain/t32-cc clean
	$(MAKE) -C vm/t32-run clean
	$(MAKE) -C vm/libt32vm clean
	$(MAKE) -C runtime/crt0 clean
	$(MAKE) -C runtime/libt32 clean
	$(MAKE) -C firmware/bios clean
	$(MAKE) -C firmware/boot clean
	$(MAKE) -C firmware/stage3 clean
	$(MAKE) -C tools/t32-disk clean

help:
	@echo T32 repository root targets:
	@echo.
	@echo   make do                 - rebuild/install t32-cc, then trusted suite
	@echo   make test               - trusted regression suite
	@echo   make test-all           - trusted suite plus tools/platform/validation
	@echo   make test-toolchain     - assembler/archive/linker/nm/compiler
	@echo   make test-vm            - libt32vm and t32-run
	@echo   make build-vm           - build libt32vm, t32-run, and t32-runx
	@echo   make build-runx         - build t32-runx only; no GUI test invoked
	@echo   make test-core          - discovered core ISA tests
	@echo   make test-architecture  - discovered architecture/SVC tests
	@echo   make test-abi           - discovered ABI tests
	@echo   make test-algorithm     - discovered algorithm tests
	@echo   make test-runtime       - crt0 and libt32
	@echo   make test-firmware      - BIOS, boot, and stage3
	@echo   make test-platform      - discovered platform/system tests
	@echo   make test-tools         - t32-disk
	@echo   make validation         - higher-level validation suite
	@echo   make list-tests         - show discovered repository tests
	@echo   make clean              - clean primary components
	@echo   make clean-tests        - clean discovered test directories

.PHONY: \
	all do prepare-cc test tests test-all \
	test-toolchain build-vm build-runx test-vm \
	test-core test-architecture test-abi test-algorithm test-platform \
	test-runtime test-firmware test-tools validation clean-validation \
	list-tests list-core-tests list-architecture-tests \
	clean clean-tests help \
	$(ALL_DISCOVERED_TEST_DIRS)
