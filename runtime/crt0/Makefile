PYTHON ?= python

all:
	$(PYTHON) tools/build.py build

test: all
	$(PYTHON) tests/run_tests.py

install:
	$(PYTHON) tools/build.py install

clean:
	$(PYTHON) tools/build.py clean

.PHONY: all test install clean
