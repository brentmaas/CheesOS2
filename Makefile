TARGET := kernel
SRC := src
INCLUDE := include
LINKER := linker
BUILD := build

ifeq ($(CC),clang)
	LDFLAGS += -target i486-freestanding-elf
	CFLAGS += -target i486-freestanding-elf
	LD := clang
else
	LD := i486-elf-gcc
	CC := i486-elf-gcc
endif

ASM := nasm

LDFLAGS += -T$(LINKER)/kernel.ld -ffreestanding -nostdlib
COMMON_FLAGS += -O3 -I$(INCLUDE) -I$(INCLUDE)/libc -ffreestanding -nostdlib
ASMFLAGS += -felf32
CFLAGS += $(COMMON_FLAGS) -std=c11

rwildcard = $(foreach d, $(wildcard $1*), $(call rwildcard, $d/, $2) $(filter $(subst *, %, $2), $d))

CSRCS := $(patsubst $(SRC)/%, %, $(call rwildcard, $(SRC)/, *.c))
ASMSRCS := $(patsubst $(SRC)/%, %, $(call rwildcard, $(SRC)/, *.asm))
OBJECTS := $(CSRCS:%.c=%.o) $(ASMSRCS:%.asm=%.o)

OBJECTS := $(filter-out libc/crtn.o, $(filter-out libc/crti.o, $(OBJECTS)))

ESC := 
RED := $(ESC)[1;31m
WHITE := $(ESC)[1;37m
BLUE := $(ESC)[1;34m
YELLOW := $(ESC)[1;33m
CLEAR := $(ESC)[0m

progress = $(info $1$(CLEAR))

vpath %.o $(BUILD)/objects
vpath %.c $(SRC)
vpath %.asm $(SRC)

all: $(TARGET)
	@echo Done!

$(TARGET): $(OBJECTS) libc/crti.o libc/crtn.o
	@$(call progress,$(RED)Linking $@)
	@$(LD) -o $@ $(BUILD)/objects/libc/crti.o $(OBJECTS:%=$(BUILD)/objects/%) $(BUILD)/objects/libc/crtn.o $(LDFLAGS)

%.o: %.c
	@$(call progress,$(BLUE)Compiling $<)
	@mkdir -p $(BUILD)/objects/$(dir $@)
	@$(CC) $(CFLAGS) -o $(BUILD)/objects/$@ -c $<

%.o: %.asm
	@$(call progress,$(BLUE)Assembling $<)
	@mkdir -p $(BUILD)/objects/$(dir $@)
	@$(ASM) $(ASMFLAGS) -o $(BUILD)/objects/$@ $<

clean:
	@echo Cleaning build files
	@rm -rf $(BUILD) $(TARGET)

run: all
	@qemu-system-x86_64 -cpu 486 -kernel $(TARGET)

.PHONY: clean
