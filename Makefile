TARGET := kernel
SRC := src
INCLUDE := include
LINKER := linker
BUILD := build
RES := res

CC := i486-unknown-elf-gcc
LD := $(CC)

ASM := nasm
ASMFLAGS += -felf32

LDFLAGS += -T $(LINKER)/kernel.ld -ffreestanding -nostdlib -flto

CFLAGS += \
	-g \
	-O2 \
	-I$(INCLUDE) \
	-I$(INCLUDE)/libc \
	-I$(BUILD)/gen \
	-ffreestanding \
	-nostdlib \
	-std=gnu11 \
	-Wall \
	-Wextra \
	-Wno-unused-function \
	-Wno-unused-parameter \
	-Wno-unused-const-variable

QEMU ?= qemu-system-x86_64
QEMU_COMMON_FLAGS += -no-reboot -cpu 486 -serial stdio -m 12M
QEMU_DEBUG_FLAGS += $(QEMU_COMMON_FLAGS) -gdb tcp::1234 -S -d int

find = $(shell find $1 -type f -name $2 -print 2> /dev/null)

FONTS = $(call find, $(RES)/fonts, "*.png")

CSRCS = $(call find, $(SRC)/, "*.c")
ASMSRCS = $(filter-out $(SRC)/libc/crti.asm $(SRC)/libc/crtn.asm,$(call find, $(SRC)/, "*.asm"))
OBJECTS = $(BUILD)/gen/res/fonts.o $(CSRCS:%=$(BUILD)/objects/%.o) $(ASMSRCS:%=$(BUILD)/objects/%.o)

all: $(BUILD)/target/$(TARGET)

$(BUILD)/target/$(TARGET): $(BUILD)/objects/src/libc/crti.asm.o $(OBJECTS) $(BUILD)/objects/src/libc/crtn.asm.o
	@echo Linking $@
	@mkdir -p $(BUILD)/target
	@$(LD) -o $@ $(BUILD)/objects/src/libc/crti.asm.o $(OBJECTS) $(BUILD)/objects/src/libc/crtn.asm.o $(LDFLAGS)

$(BUILD)/objects/%.c.o: %.c $(BUILD)/gen/res/fonts.o
	@echo Compiling $(subst $(BUILD)/,,$<)
	@mkdir -p $(dir $@)
	@$(CC) -MMD $(CFLAGS) -c -o $@ $<

$(BUILD)/objects/%.asm.o: %.asm
	@echo Assembling $(subst $(BUILD)/,,$<)
	@mkdir -p $(dir $@)
	@$(ASM) -MD $@.d $(ASMFLAGS) -o $@ $<

$(BUILD)/gen/res/fonts.o: $(FONTS)
	@echo Generating fonts
	@mkdir -p $(dir $@)
	@python3 tools/createcharmap.py \
		-C $(BUILD)/gen/res/fonts.c \
		-H $(BUILD)/gen/res/fonts.h \
		-I res/fonts.h \
		$(FONTS)
	@$(CC) $(CFLAGS) -c -o $@ $(BUILD)/gen/res/fonts.c

$(BUILD)/gen/res/fonts.h: $(BUILD)/gen/res/fonts.o

clean:
	@echo Cleaning build files
	@rm -rf $(BUILD)

run: $(BUILD)/target/$(TARGET)
	@$(QEMU) $(QEMU_COMMON_FLAGS) -kernel $<

run-debug: $(BUILD)/target/$(TARGET)
	@$(QEMU) $(QEMU_DEBUG_FLAGS) -kernel $<

-include $(call find, $(BUILD)/, "*.d")

.PHONY: clean run run-debug
