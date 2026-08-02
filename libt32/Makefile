PYTHON := python

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

.PHONY: all check objects archive inspect test clean
