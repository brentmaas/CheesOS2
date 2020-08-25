#include "interrupt/exceptions.h"
#include "interrupt/idt.h"
#include "core/panic.h"
#include "debug/log.h"

#include <stdint.h>

const char* INTERRUPT_NAMES[] = {
    [IDT_EXCEPTION_DIVIDE_ERROR] = "Divide By Zero",
    [IDT_EXCEPTION_DEBUG] = "Debug",
    [IDT_EXCEPTION_NMI] = "Non-Maskable Interrupt",
    [IDT_EXCEPTION_BREAKPOINT] = "Breakpoint",
    [IDT_EXCEPTION_OVERFLOW] = "Overflow",
    [IDT_EXCEPTION_BOUND_RANGE] = "Bound Range Exceeded",
    [IDT_EXCEPTION_INVALID_OPCODE] = "Invalid Opcode",
    [IDT_EXCEPTION_DEVICE_NOT_AVAILABLE] = "Device Not Available",
    [IDT_EXCEPTION_DOUBLE_FAULT] = "Double Fault",
    [IDT_EXCEPTION_COPROC_SEGMENT_OVR] = "Coprocessor Segment Overrun",
    [IDT_EXCEPTION_INVALID_TSS] = "Invalid TSS",
    [IDT_EXCEPTION_SEGMENT_NOT_PRESENT] = "Segment Not Present",
    [IDT_EXCEPTION_STACK_SEGMENT_FAULT] = "Stack-Segment Fault",
    [IDT_EXCEPTION_GENERAL_PROTECTION_FAULT] = "General Protection Fault",
    [IDT_EXCEPTION_PAGE_FAULT] = "Page Fault",
    [IDT_EXCEPTION_RESERVED_15] = "Reserved",
    [IDT_EXCEPTION_X87] = "x87 FPU Exception",
    [IDT_EXCEPTION_ALIGNMENT] = "Alignment Check",
    [IDT_EXCEPTION_MACHINE_CHECK] = "Machine Check",
    [IDT_EXCEPTION_SIMD] = "SIMD Floating-Point Exception",
    [IDT_EXCEPTION_VIRTUALIZATION] = "Virtualization Exception",
    [IDT_EXCEPTION_CONTROL_PROTECTION] = "Control Protection Exception"
};

void idt_exception_dump_registers(struct interrupt_registers* registers, struct interrupt_parameters* parameters) {
    log_debug("Register dump:\n"
        "EAX = 0x%08X\tECX = 0x%08X\n"
        "EDX = 0x%08X\tEBX = 0x%08X\n"
        "ESP = 0x%08X\tEBP = 0x%08X\n"
        "ESI = 0x%08X\tEDI = 0x%08X\n"
        "DS = 0x%04X\t\t\tES = 0x%04X\n"
        "FS = 0x%04X\t\t\tGS = 0x%04X\n"
        "\n"
        "EIP = 0x%08X\tCS = 0x%04X\n"
        "EFLAGS = 0x%08X\n",
        registers->eax, registers->ecx,
        registers->edx, registers->ebx,
        registers->esp, registers->ebp,
        registers->esi, registers->edi,
        registers->ds, registers->es,
        registers->fs, registers->gs,
        parameters->eip, (uint32_t)parameters->cs,
        parameters->eflags
    );
}

void idt_exception_no_status(uint32_t interrupt, struct interrupt_registers* registers, struct interrupt_parameters* parameters) {
    log_error("Hardware exception %u (%s)", interrupt, INTERRUPT_NAMES[interrupt]);
    idt_exception_dump_registers(registers, parameters);
    kernel_panic();
}

void idt_exception_status(uint32_t interrupt, struct interrupt_registers* registers, struct interrupt_parameters* parameters, uint32_t status) {
    log_error("Hardware exception %u (%s); status = %u (0x%X)", interrupt, INTERRUPT_NAMES[interrupt], status, status);
    if (interrupt == IDT_EXCEPTION_PAGE_FAULT) {
        uint32_t cr2;
        asm volatile ("mov %%cr2, %0" : "=r"(cr2));
        log_error("While accessing virtual address %p", cr2);
    }

    idt_exception_dump_registers(registers, parameters);
    kernel_panic();
}

void idt_exceptions_load(void) {
    idt_make_interrupt_no_status(IDT_EXCEPTION_DIVIDE_ERROR, idt_exception_no_status, IDT_GATE_TYPE_INTERRUPT_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    //TODO: Debug
    //TODO: Non-maskable interrupt
    idt_make_interrupt_no_status(IDT_EXCEPTION_BREAKPOINT, idt_exception_no_status, IDT_GATE_TYPE_INTERRUPT_32, IDT_PRIVILEGE_3 | IDT_FLAG_PRESENT);
    idt_make_interrupt_no_status(IDT_EXCEPTION_OVERFLOW, idt_exception_no_status, IDT_GATE_TYPE_INTERRUPT_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_no_status(IDT_EXCEPTION_BOUND_RANGE, idt_exception_no_status, IDT_GATE_TYPE_INTERRUPT_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_no_status(IDT_EXCEPTION_INVALID_OPCODE, idt_exception_no_status, IDT_GATE_TYPE_INTERRUPT_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_no_status(IDT_EXCEPTION_DEVICE_NOT_AVAILABLE, idt_exception_no_status, IDT_GATE_TYPE_INTERRUPT_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_status(IDT_EXCEPTION_DOUBLE_FAULT, idt_exception_status, IDT_GATE_TYPE_INTERRUPT_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_no_status(IDT_EXCEPTION_COPROC_SEGMENT_OVR, idt_exception_no_status, IDT_GATE_TYPE_INTERRUPT_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_status(IDT_EXCEPTION_INVALID_TSS, idt_exception_status, IDT_GATE_TYPE_INTERRUPT_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_status(IDT_EXCEPTION_SEGMENT_NOT_PRESENT, idt_exception_status, IDT_GATE_TYPE_INTERRUPT_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_status(IDT_EXCEPTION_STACK_SEGMENT_FAULT, idt_exception_status, IDT_GATE_TYPE_INTERRUPT_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_status(IDT_EXCEPTION_GENERAL_PROTECTION_FAULT, idt_exception_status, IDT_GATE_TYPE_INTERRUPT_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_status(IDT_EXCEPTION_PAGE_FAULT, idt_exception_status, IDT_GATE_TYPE_INTERRUPT_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    //TODO: Reserved
    idt_make_interrupt_no_status(IDT_EXCEPTION_X87, idt_exception_no_status, IDT_GATE_TYPE_INTERRUPT_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_status(IDT_EXCEPTION_ALIGNMENT, idt_exception_status, IDT_GATE_TYPE_INTERRUPT_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    idt_make_interrupt_no_status(IDT_EXCEPTION_MACHINE_CHECK, idt_exception_no_status, IDT_GATE_TYPE_INTERRUPT_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
    //TODO: SIMD Floating-Point Exception
    //TODO: Virtualization Exception
    idt_make_interrupt_no_status(IDT_EXCEPTION_CONTROL_PROTECTION, idt_exception_no_status, IDT_GATE_TYPE_INTERRUPT_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT);
}