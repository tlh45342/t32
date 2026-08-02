CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -O2

TARGET := t32-ar.exe
OBJECT := build/t32-ar.o
SOURCE := src/t32-ar.c

all: $(TARGET)

$(TARGET): $(OBJECT)
	$(CC) $(OBJECT) -o $(TARGET)

$(OBJECT): $(SOURCE)
	@if not exist build mkdir build
	$(CC) $(CFLAGS) -c $(SOURCE) -o $(OBJECT)

test: $(TARGET)
	python tests/run_tests.py

install: $(TARGET)
	@if not exist "$(USERPROFILE)\.local\bin" mkdir "$(USERPROFILE)\.local\bin"
	copy /Y $(TARGET) "$(USERPROFILE)\.local\bin\$(TARGET)"
	@echo Installed $(TARGET) to $(USERPROFILE)\.local\bin

clean:
	@if exist build rmdir /s /q build
	@if exist $(TARGET) del /q $(TARGET)

.PHONY: all test install clean
