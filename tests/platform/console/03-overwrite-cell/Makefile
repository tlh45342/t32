ASM ?= t32-as
RUN ?= t32-run

all: overwrite-cell.bin

overwrite-cell.bin: overwrite-cell.s
	$(ASM) -f bin overwrite-cell.s -o overwrite-cell.bin

test: overwrite-cell.bin
	$(RUN) < test.script

clean:
	rm -f overwrite-cell.bin overwrite-cell.log

.PHONY: all test clean
