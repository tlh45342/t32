T32_AS ?= t32-as
T32_RUN ?= ../../../vm/t32-run/bin/t32-run.exe
PYTHON ?= python

.PHONY: all test clean

all: rtc.bin

rtc.bin: rtc.s
	$(T32_AS) -f bin rtc.s -o rtc.bin

test: rtc.bin
	$(PYTHON) run_tests.py "$(T32_RUN)" "rtc.bin"

clean:
	@if exist rtc.bin del /Q rtc.bin
