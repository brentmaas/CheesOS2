#include <stdint.h>
#include <stddef.h>

#include "core/multiboot.h"
#include "core/panic.h"
#include "interrupt/idt.h"
#include "interrupt/pic.h"

#include "memory/gdt.h"
#include "memory/pmm.h"
#include "memory/vmm.h"

#include "driver/vga/text.h"
#include "driver/serial/serial.h"

#include "ps2/controller.h"
#include "ps2/device.h"

#include "debug/log.h"
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

static void syscall_handler(uint32_t interrupt, struct interrupt_registers* registers, struct interrupt_parameters* parameters) {
    log_info("Syscall interrupt");
}

static void test_usermode() {
    asm volatile ("int $'B'");
    *(volatile int*)0;
}

void kernel_main(const struct multiboot* multiboot) {
    vmm_unmap_identity();

    serial_init(SERIAL_PORT_1, ((struct serial_init_info) {
        .data_size = SERIAL_DATA_SIZE_8_BITS,
        .stop_bits = SERIAL_STOP_BITS_ONE,
        .parity = SERIAL_PARITY_NONE,
        .enable_break = false,
        .baudrate_divisor = SERIAL_BAUDRATE_DIVISOR(9600)
    }));

    log_set_sink(sink_serial, NULL);

    log_info("Initializing GDT");
    gdt_init();

    log_info("Initializing IDT");
    idt_init();
    pic_remap(0x20, 0x28);
    pic_set_mask(0xEFF9);
    ps2_device_register_interrupts(0x21, 0x2C);
    idt_enable();

    log_info("Initializing console");
    console_init();
    log_set_sink(sink_serial_and_console, NULL);

    if (multiboot->flags & MULTIBOOT_FLAG_BOOT_LOADER_NAME) {
        log_info("Booted from %s", multiboot->boot_loader_name);
    }
    if (multiboot->flags & MULTIBOOT_FLAG_CMDLINE) {
        log_info("Booted with command line \"%s\"", multiboot->cmdline);
    }

    pmm_init(multiboot);

    // if (ps2_controller_init()) {
    //     log_error("PS2 initialization failed");
    //     return;
    // }

    log_info("Initialization finished");

    console_print("OPPERPYTHON\n");
    console_set_attr(VGA_ATTR_GREEN, VGA_ATTR_RED);
    console_print("IS\n");
    console_set_attr(VGA_ATTR_BLUE, VGA_ATTR_WHITE);
    console_print("GOED\n");
    console_set_attr(VGA_ATTR_GREEN, VGA_ATTR_BLACK);
    console_print("\x90\x91\x91\x91\x91\x91\x91\x91\x91\x92\n");
    console_set_attr(VGA_ATTR_WHITE, VGA_ATTR_BLACK);

    idt_disable();
    idt_make_interrupt_no_status('B', syscall_handler, IDT_GATE_TYPE_INTERRUPT_32, IDT_PRIVILEGE_3 | IDT_FLAG_PRESENT);
    gdt_set_int_stack((void*) 0xC0001000);
    idt_enable();

    log_info("Jumping to usercode at %p", (void*) test_usermode);
    gdt_jump_to_usermode((void*) test_usermode, (void*) 0xC0000000);
}
