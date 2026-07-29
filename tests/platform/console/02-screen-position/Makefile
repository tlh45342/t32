ASM ?= t32-as
RUN ?= t32-run

all: screen-position.bin

screen-position.bin: screen-position.s
	$(ASM) -f bin screen-position.s -o screen-position.bin

test: screen-position.bin
	$(RUN) < test.script

clean:
	rm -f screen-position.bin screen-position.log

.PHONY: all test clean
