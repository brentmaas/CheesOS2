TARGET := kernel.elf
SRC := src
INCLUDE := include
BUILD := build
CFLAGS += -I$(INCLUDE) -g -std=gnu11 -Wall -Wextra -O3 -nostdlib
ASFLAGS += -felf64
LDFLAGS := -nostdlib
CC := x86_64-elf-gcc
LD := x86_64-elf-gcc
AS := nasm

find = $(shell find $1 -type f -name $2 -print 2>/dev/null)
objects = $(patsubst %.c, $(BUILD)/objects/%.c.o, $(call find, $1, "*.c")) $(patsubst %.asm, $(BUILD)/objects/%.asm.o, $(call find, $1, "*.asm"))

OBJ = $(call objects, $(SRC)/)

ESC := 
RED := $(ESC)[1;31m
BLUE := $(ESC)[1;34m
CLEAR := $(ESC)[0m

progress = $(info $1$(CLEAR))

all: $(TARGET)

$(TARGET): $(OBJ)
	@$(call progress,$(RED)Linking $@)
	@mkdir -p $(BUILD)/target
	@$(LD) -o $@ $^ $(LDFLAGS)

$(BUILD)/objects/%.c.o: %.c
	@$(call progress,$(BLUE)Compiling $<)
	@mkdir -p $(dir $@)
	@$(CC) -MMD -c $(CFLAGS) -o $@ $<

$(BUILD)/objects/%.asm.o: %.asm
	@$(call progress,$(BLUE)Assembling $<)
	@mkdir -p $(dir $@)
	@$(AS) $(ASFLAGS) -o $@ $<

clean:
	@echo Cleaning build files
	@rm -rf $(BUILD)
	@rm -f $(TARGET)

-include $(call find, $(BUILD)/, "*.d")

.PHONY: clean
