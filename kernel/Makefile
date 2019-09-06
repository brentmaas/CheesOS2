TARGET := kernel.elf
SRC := src
INCLUDE := include
BUILD := build
LINK := link
COMMON_FLAGS := -I$(INCLUDE) -ffreestanding -mcmodel=large -mno-red-zone -Wall -Wextra -O3 -nostdlib
CFLAGS += $(COMMON_FLAGS) -std=gnu11
CPPFLAGS += $(COMMON_FLAGS) -std=gnu++17
ASFLAGS += -felf64
LDFLAGS := -nostdlib
CC := x86_64-elf-gcc
CXX := x86_64-elf-g++
LD := x86_64-elf-gcc
AS := nasm

COMMON_FLAGS := -Iinclude -Wall -Wextra -O3 -nostdlib -Wno-unused-parameter
CFLAGS += $(COMMON_FLAGS) -std=gnu11
CPPFLAGS += $(COMMON_FLAGS) -std=gnu++17
ASFLAGS += -felf64
LDFLAGS := -nostdlib

CRTBEGIN_OBJ := $(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ := $(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
CRTI_OBJ := $(BUILD)/link/crti.asm.o
CRTN_OBJ := $(BUILD)/link/crtn.asm.o

KERNEL_SRC = $(shell find src/ -type f -regex ".*\.\(asm\|cpp\|c\)")
KERNEL_OBJ = $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(KERNEL_SRC:%=$(BUILD)/%.o) $(CRTEND_OBJ) $(CRTN_OBJ)

all: $(BUILD)/$(TARGET)

$(BUILD)/$(TARGET): $(KERNEL_OBJ)
	@echo [Kernel] Linking $(subst $(BUILD)/,,$@)
	@mkdir -p $(BUILD)/target
	@$(LD) -o $@ $^ $(LDFLAGS)

$(BUILD)/%.c.o: %.c
	@echo [Kernel] Compiling $(subst $(BUILD)/,,$<)
	@mkdir -p $(dir $@)
	@$(CC) -MMD -c $(CFLAGS) -o $@ $<

$(BUILD)/%.asm.o: %.asm
	@echo [Kernel] Assembling $(subst $(BUILD)/,,$<)
	@mkdir -p $(dir $@)
	@$(AS) $(ASFLAGS) -o $@ $<

$(BUILD)/%.cpp.o: %.cpp
	@echo [Kernel] Compiling $(subst $(BUILD)/,,$<)
	@mkdir -p $(dir $@)
	@$(CXX) -MMD -c $(CPPFLAGS) -o $@ $<

clean:
	@rm -rf build
	@rm -f $(CRTI_OBJ) $(CRTN_OBJ)

-include $(shell find $(BUILD)/ -type f -name "*.d" -print 2>/dev/null)

.PHONY: clean
