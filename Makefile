BUILD := $(realpath .)/build
CC := clang
AS := nasm

export

OVMF_X64 ?= /usr/share/ovmf/x64/

all: bootloader kernel

bootloader:
	@$(MAKE) -C bootloader all

kernel:
	@$(MAKE) -C kernel -f Makefile.clang all

clean:
	@rm -rf $(BUILD)

qemu: bootloader kernel
	@mkdir -p $(BUILD)/qemu/boot
	@cp $(BUILD)/bootloader/bootloader.efi $(BUILD)/qemu/boot/
	@cp $(BUILD)/kernel/kernel.elf $(BUILD)/qemu/boot
	@qemu-system-x86_64 \
		-drive if=pflash,format=raw,unit=0,file=$(OVMF_X64)/OVMF_CODE.fd,readonly=on \
		-drive if=pflash,format=raw,unit=1,file=$(OVMF_X64)/OVMF_VARS.fd,readonly=on \
		-net none \
		-nographic \
		-hdd fat:rw:build/qemu/boot/

.PHONY: bootloader kernel clean qemu