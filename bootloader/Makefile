TARGET := bootloader.efi
BUILD := $(BUILD)/bootloader
CC := clang
LD := lld-link
CFLAGS += \
	-std=gnu11 \
	-target x86_64-unknown-windows \
	-fno-stack-protector \
	-ffreestanding \
	-fshort-wchar \
	-fno-PIC \
	-mno-red-zone \
	-mno-mmx \
	-mno-sse \
	-isystem include/c-efi \
	-Wall \
	-Wextra \
	-Wno-unused-parameter \
	-O3 \
	-g \
	-nostdlib \

LDFLAGS += \
	-nodefaultlib \
	-entry:efi_main \
	-subsystem:efi_application

BOOTLOADER_SRC := $(shell find src/ -type f -name "*.c" -print)
BOOTLOADER_OBJ := $(BOOTLOADER_SRC:%=$(BUILD)/%.o)

all: $(BUILD)/$(TARGET)

$(BUILD)/$(TARGET): $(BOOTLOADER_OBJ)
	@echo [Bootloader] Linking $(subst $(BUILD)/,,$@)
	@mkdir -p $(dir $@)
	@$(LD) $(LDFLAGS) -out:$@ $^

$(BUILD)/%.c.o: %.c
	@echo [Bootloader] Compiling $(subst $(BUILD)/,,$<)
	@mkdir -p $(dir $@)
	@$(CC) -MMD $(CFLAGS) -c -o $@ $<

-include $(shell find $(BUILD)/ -type f -name "*.d" -print 2>/dev/null)
