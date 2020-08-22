#include <stdint.h>
#include <stddef.h>

#include "memory/gdt.h"
#include "interrupt/idt.h"
#include "core/multiboot.h"
#include "interrupt/pic.h"

#include "driver/vga/text.h"

#include "driver/serial/serial.h"

#include "ps2/controller.h"

#include "debug/log.h"
#include "debug/assert.h"
#include "debug/console/console.h"
#include "debug/console/shell.h"

static int sink_serial_cprintf_cbk(void* context, const char* data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        serial_busy_write(SERIAL_PORT_1, data[i]);
    }
    return 0;
}

static void sink_serial(void* context, enum log_level level, const char* file, unsigned line, const char* format, va_list args) {
    cprintf(sink_serial_cprintf_cbk, NULL, "%s %s:%u: ", LOG_LEVEL_NAMES[level], file, line);
    vcprintf(sink_serial_cprintf_cbk, NULL, format, args);
    serial_busy_write(SERIAL_PORT_1, '\n');
}

static void sink_serial_and_console(void* context, enum log_level level, const char* file, unsigned line, const char* format, va_list args) {
    sink_serial(NULL, level, file, line, format, args);
    console_log_sink(NULL, level, file, line, format, args);
}

void kernel_main(const struct multiboot_info* multiboot) {
    serial_init(SERIAL_PORT_1, ((struct serial_init_info) {
        .data_size = SERIAL_DATA_SIZE_8_BITS,
        .stop_bits = SERIAL_STOP_BITS_ONE,
        .parity = SERIAL_PARITY_NONE,
        .enable_break = false,
        .baudrate_divisor = SERIAL_BAUDRATE_DIVISOR(9600)
    }));

    log_set_sink(sink_serial, NULL);
    log_info("Initializing console");
    console_init();

    log_set_sink(sink_serial_and_console, NULL);

    log_info("Initializing GDT");
    gdt_init();

    log_info("Initializing IDT");
    idt_init();
    pic_remap(0x20, 0x28);
    pic_set_mask(0xFFFF); //Disable PIC
    idt_enable();

    if (multiboot->flags & MULTIBOOT_FLAG_BOOT_LOADER_NAME) {
        log_info("Booted from %s", multiboot->boot_loader_name);
    }
    if (multiboot->flags & MULTIBOOT_FLAG_CMDLINE) {
        log_info("Booted with command line \"%s\"", multiboot->cmdline);
    }

    if (ps2_controller_init()) {
        log_error("PS2 initialization failed");
        return;
    }

    console_print("OPPERPYTHON\n");
    console_set_attr(VGA_ATTR_GREEN, VGA_ATTR_RED);
    console_print("IS\n");
    console_set_attr(VGA_ATTR_BLUE, VGA_ATTR_WHITE);
    console_print("GOED\n");
    console_set_attr(VGA_ATTR_GREEN, VGA_ATTR_BLACK);
    console_print("\x90\x91\x91\x91\x91\x91\x91\x91\x91\x92\n");
    console_set_attr(VGA_ATTR_WHITE, VGA_ATTR_BLACK);

    console_shell_loop();
}
