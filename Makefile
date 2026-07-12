# T32 test-suite Makefile
#
# This top-level Makefile does not build t32-run or t32-asm.
# It discovers test directories and asks each test directory to run
# its own local "make test" target.
#
# Expected layout:
#
#   tests/
#     core-iset/
#       01-movi/
#         Makefile
#       02-mov/
#         Makefile
#
# Commands:
#
#   make test
#   make tests
#   make clean
#   make list-tests

# Find each test directory containing a Makefile.
#
# This matches:
#   tests/<test-group>/<test-name>/Makefile
#
# Examples:
#   tests/core-iset/01-movi/Makefile
#   tests/core-iset/02-mov/Makefile
TEST_MAKEFILES := $(wildcard tests/*/*/Makefile)
TEST_DIRS      := $(sort $(patsubst %/Makefile,%,$(TEST_MAKEFILES)))

all: test

# "test" and "tests" are aliases.
test tests: $(TEST_DIRS)
	@echo
	@echo "All T32 tests passed."

# Each discovered test directory becomes a recursive make target.
$(TEST_DIRS):
	@echo
	@echo "============================================================"
	@echo "Running test: $@"
	@echo "============================================================"
	$(MAKE) -C "$@" test

list-tests:
	@echo "Discovered T32 tests:"
	@$(foreach dir,$(TEST_DIRS),echo "  $(dir)";)

clean:
	@$(foreach dir,$(TEST_DIRS),$(MAKE) -C "$(dir)" clean || exit 1;)
	@echo "T32 test artifacts removed."

.PHONY: all test tests list-tests clean $(TEST_DIRS)
