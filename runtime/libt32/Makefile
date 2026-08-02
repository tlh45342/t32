PYTHON := python

PREFIX ?= $(USERPROFILE)\.local
T32_LIBDIR ?= $(PREFIX)\lib\t32

all: archive

check:
	$(PYTHON) tools/check_manifest.py

objects: check
	$(PYTHON) tools/build.py objects

archive: check
	$(PYTHON) tools/build.py archive

inspect: check
	$(PYTHON) tools/build.py inspect

test: archive
	$(PYTHON) tests/run_tests.py

clean:
	$(PYTHON) tools/build.py clean

install: archive
	@if not exist "$(T32_LIBDIR)" mkdir "$(T32_LIBDIR)"
	copy /Y build\libt32.a "$(T32_LIBDIR)\libt32.a"
	@echo Installed T32 target library to $(T32_LIBDIR)\libt32.a

.PHONY: all objects archive inspect test install clean
