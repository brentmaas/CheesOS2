TARGET := bootloader.efi
BUILD := $(BUILD)/bootloader
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
	-Iinclude/c-efi \
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
BOOTLOADER_OBJ := $(BOOTLOADER_SRC:%=$(BUILD)/object/%.o)

all: $(BUILD)/$(TARGET)

$(BUILD)/$(TARGET): $(BOOTLOADER_OBJ)
	@mkdir -p $(dir $@)
	@$(LD) $(LDFLAGS) -out:$@ $^

$(BUILD)/object/%.c.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c -o $@ $^
