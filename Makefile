ifeq ($(OS),Windows_NT)
	HOST_WINDOWS := 1
else
	HOST_WINDOWS := 0
endif

AS_DIR:=toolchain/t32-as
AR_DIR:=toolchain/t32-ar
LD_DIR:=toolchain/t32-ld
NM_DIR:=toolchain/t32-nm
CC_DIR:=toolchain/t32-cc
CRT0_DIR:=runtime/crt0
LIBT32_DIR:=runtime/libt32
DISK_TOOL_DIR:=tools/t32-disk
LIBT32VM_DIR:=vm/libt32vm
RUN_DIR:=vm/t32-run
RUNX_DIR:=vm/t32-runx
BIOS_DIR:=firmware/bios
BOOT_DIR:=firmware/boot
STAGE3_DIR:=firmware/stage3
VALID_DIR:=validation
SUBMAKE:=$(MAKE) --no-print-directory

.PHONY: all toolchain runtime tools vm firmware validation \
test test-toolchain test-runtime test-tools test-vm test-firmware \
install install-toolchain install-runtime install-tools install-vm install-firmware \
clean clean-toolchain clean-runtime clean-tools clean-vm clean-firmware

all: toolchain runtime tools vm firmware
toolchain:
	$(SUBMAKE) -C $(AS_DIR)
	$(SUBMAKE) -C $(AR_DIR)
	$(SUBMAKE) -C $(LD_DIR)
	$(SUBMAKE) -C $(NM_DIR)
	$(SUBMAKE) -C $(CC_DIR)
runtime:
	$(SUBMAKE) -C $(CRT0_DIR)
	$(SUBMAKE) -C $(LIBT32_DIR)
tools:
	$(SUBMAKE) -C $(DISK_TOOL_DIR)
vm:
	$(SUBMAKE) -C $(LIBT32VM_DIR)
	$(SUBMAKE) -C $(RUN_DIR)
ifeq ($(HOST_WINDOWS),1)
	$(SUBMAKE) -C $(RUNX_DIR)
endif
firmware:
	$(SUBMAKE) -C $(BIOS_DIR)
	$(SUBMAKE) -C $(BOOT_DIR)
	$(SUBMAKE) -C $(STAGE3_DIR)
validation:
	$(SUBMAKE) -C $(VALID_DIR) test

test: test-toolchain test-runtime test-tools test-vm test-firmware validation
test-toolchain:
	$(SUBMAKE) -C $(AS_DIR) test
	$(SUBMAKE) -C $(AR_DIR) test
	$(SUBMAKE) -C $(LD_DIR) test
	$(SUBMAKE) -C $(NM_DIR) test
	$(SUBMAKE) -C $(CC_DIR) test
test-runtime:
	$(SUBMAKE) -C $(CRT0_DIR) test
	$(SUBMAKE) -C $(LIBT32_DIR) test
test-tools:
	$(SUBMAKE) -C $(DISK_TOOL_DIR) test
test-vm:
	$(SUBMAKE) -C $(LIBT32VM_DIR) test
	$(SUBMAKE) -C $(RUN_DIR) test
ifeq ($(HOST_WINDOWS),1)
	$(SUBMAKE) -C $(RUNX_DIR) test
endif
test-firmware:
	$(SUBMAKE) -C $(BIOS_DIR) test
	$(SUBMAKE) -C $(BOOT_DIR) test
	$(SUBMAKE) -C $(STAGE3_DIR) test

install: install-toolchain install-runtime install-tools install-vm install-firmware
install-toolchain:
	$(SUBMAKE) -C $(AS_DIR) install
	$(SUBMAKE) -C $(AR_DIR) install
	$(SUBMAKE) -C $(LD_DIR) install
	$(SUBMAKE) -C $(NM_DIR) install
	$(SUBMAKE) -C $(CC_DIR) install
install-runtime:
	$(SUBMAKE) -C $(CRT0_DIR) install
	$(SUBMAKE) -C $(LIBT32_DIR) install
install-tools:
	$(SUBMAKE) -C $(DISK_TOOL_DIR) install
install-vm:
	$(SUBMAKE) -C $(LIBT32VM_DIR) install
	$(SUBMAKE) -C $(RUN_DIR) install
ifeq ($(HOST_WINDOWS),1)
	$(SUBMAKE) -C $(RUNX_DIR) install
endif
install-firmware: firmware
	$(SUBMAKE) -C $(BOOT_DIR) install
	$(SUBMAKE) -C $(STAGE3_DIR) install
ifeq ($(OS),Windows_NT)
	@if not exist "$(USERPROFILE)\.local\share\t32\firmware" mkdir "$(USERPROFILE)\.local\share\t32\firmware"
	copy /Y "$(subst /,\,$(BIOS_DIR)/bios.bin)" "$(USERPROFILE)\.local\share\t32\firmware\bios.bin" >NUL
else
	mkdir -p "$(HOME)/.local/share/t32/firmware"
	cp -f "$(BIOS_DIR)/bios.bin" "$(HOME)/.local/share/t32/firmware/bios.bin"
endif

clean: clean-toolchain clean-runtime clean-tools clean-vm clean-firmware
clean-toolchain:
	$(SUBMAKE) -C $(AS_DIR) clean
	$(SUBMAKE) -C $(AR_DIR) clean
	$(SUBMAKE) -C $(LD_DIR) clean
	$(SUBMAKE) -C $(NM_DIR) clean
	$(SUBMAKE) -C $(CC_DIR) clean
clean-runtime:
	$(SUBMAKE) -C $(CRT0_DIR) clean
	$(SUBMAKE) -C $(LIBT32_DIR) clean
clean-tools:
	$(SUBMAKE) -C $(DISK_TOOL_DIR) clean
clean-vm:
ifeq ($(HOST_WINDOWS),1)
	$(SUBMAKE) -C $(RUNX_DIR) clean
endif
	$(SUBMAKE) -C $(RUN_DIR) clean
	$(SUBMAKE) -C $(LIBT32VM_DIR) clean
clean-firmware:
	$(SUBMAKE) -C $(STAGE3_DIR) clean
	$(SUBMAKE) -C $(BOOT_DIR) clean
	$(SUBMAKE) -C $(BIOS_DIR) clean
