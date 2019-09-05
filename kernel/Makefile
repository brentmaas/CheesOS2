TARGET := kernel.elf
SRC := src
INCLUDE := include
BUILD := build
LINK := link
COMMON_FLAGS := -I$(INCLUDE) -Wall -Wextra -O3 -nostdlib
CFLAGS += $(COMMON_FLAGS) -std=gnu11
CPPFLAGS += $(COMMON_FLAGS) -std=gnu++17
ASFLAGS += -felf64
LDFLAGS := -nostdlib
CC := x86_64-elf-gcc
CXX := x86_64-elf-g++
LD := x86_64-elf-gcc
AS := nasm

CRTBEGIN_OBJ := $(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ := $(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
CRTI_OBJ := $(LINK)/crti.o
CRTN_OBJ := $(LINK)/crtn.o

find = $(shell find $1 -type f -name $2 -print 2>/dev/null)
objects = $(patsubst %.c, $(BUILD)/objects/%.c.o, $(call find, $1, "*.c")) $(patsubst %.asm, $(BUILD)/objects/%.asm.o, $(call find, $1, "*.asm")) $(patsubst %.cpp, $(BUILD)/objects/%.cpp.o, $(call find, $1, "*.cpp")) 

OBJ = $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(call objects, $(SRC)/) $(CRTEND_OBJ) $(CRTN_OBJ)

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

$(BUILD)/objects/%.cpp.o: %.cpp
	@$(call progress,$(BLUE)Compiling $<)
	@mkdir -p $(dir $@)
	@$(CXX) -MMD -c $(CPPFLAGS) -o $@ $<

$(LINK)/%.o: $(LINK)/%.asm
	@$(call progress,$(BLUE)Assembling $<)
	@$(AS) $(ASFLAGS) -o $@ $<

clean:
	@echo Cleaning build files
	@rm -rf $(BUILD)
	@rm -f $(TARGET)
	@rm -f $(CRTI_OBJ)
	@rm -f $(CRTN_OBJ)

-include $(call find, $(BUILD)/, "*.d")

.PHONY: clean
