TARGET := kernel
SRC := src
INCLUDE := include
LINKER := linker
BUILD := build

LD := i486-elf-gcc
CC := i486-elf-gcc

ASM := nasm
ASMFLAGS += -felf32

LDFLAGS += -T$(LINKER)/kernel.ld -ffreestanding -nostdlib -flto
COMMON_FLAGS += -g -I$(INCLUDE) -I$(INCLUDE)/libc -ffreestanding -nostdlib
CFLAGS += $(COMMON_FLAGS) -std=gnu11

QEMU ?= qemu-system-x86_64
QEMU_COMMON_FLAGS += -no-reboot  -cpu 486 -kernel $(TARGET)
QEMU_DEBUG_FLAGS += $(QEMU_COMMON_FLAGS) -gdb tcp::1234 -S -d int

find = $(shell find $1 -type f -name $2 -print 2> /dev/null)

CSRCS = $(call find, $(SRC)/, "*.c")
ASMSRCS = $(filter-out src/libc/ctri.asm src/libc/ctrn.asm,$(call find, $(SRC)/, "*.asm"))
OBJECTS = $(CSRCS:%=$(BUILD)/objects/%.o) $(ASMSRCS:%=$(BUILD)/objects/%.o)

all: $(BUILD)/target/$(TARGET)

$(BUILD)/target/$(TARGET): $(BUILD)/objects/src/libc/crti.asm.o $(OBJECTS) $(BUILD)/objects/src/libc/crtn.asm.o
	@echo Linking $@
	@mkdir -p $(BUILD)/target
	@$(LD) -o $@ $^ $(LDFLAGS)

$(BUILD)/objects/%.c.o: %.c
	@echo Compiling $(subst $(BUILD)/,,$<)
	@mkdir -p $(dir $@)
	@$(CC) -MMD $(CFLAGS) -c -o $@ $<

$(BUILD)/objects/%.asm.o: %.asm
	@echo Assembling $(subst $(BUILD)/,,$<)
	@mkdir -p $(dir $@)
	@$(ASM) -MD $@.d $(ASMFLAGS) -o $@ $<

clean:
	@echo Cleaning build files
	@rm -rf $(BUILD)

run: $(BUILD)/target/$(TARGET)
	@$(QEMU) $(QEMU_COMMON_FLAGS) -kernel $<

run-debug: $(BUILD)/target/$(TARGET)
	@$(QEMU) $(QEMU_DEBUG_FLAGS) -kernel $<

-include $(call find, $(BUILD)/, "*.d")

.PHONY: clean
