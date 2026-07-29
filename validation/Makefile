VALIDATION_MAKEFILES := $(wildcard control-flow/*/Makefile)
VALIDATION_DIRS := $(sort $(patsubst %/Makefile,%,$(VALIDATION_MAKEFILES)))

all: test

test: $(VALIDATION_DIRS)
	@echo All T32 control-flow validations passed.

$(VALIDATION_DIRS):
	@echo ============================================================
	@echo Running validation: $@
	@echo ============================================================
	$(MAKE) -C "$@" test

list:
	@$(foreach dir,$(VALIDATION_DIRS),echo $(dir);)

clean:
	@$(foreach dir,$(VALIDATION_DIRS),$(MAKE) -C "$(dir)" clean || exit 1;)

.PHONY: all test list clean $(VALIDATION_DIRS)
