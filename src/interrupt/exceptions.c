#include "interrupt/exceptions.h"
#include "interrupt/idt.h"
#include "core/panic.h"
#include "vga/vga.h"

#include <stdint.h>

const char* INTERRUPT_NAMES[] = {
    "Divide By Zero",
    "Debug",
    "Non-Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Availlable",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 FPU Exception",
    "Alignemnt Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception"
};

void idt_exception_dump_registers(struct interrupt_registers* registers, struct interrupt_parameters* parameters) {
    vga_printf("EAX = 0x%08x\tECX = 0x%08x\n"
        "EDX = 0x%08x\tEBX = 0x%08x\n"
        "ESP = 0x%08x\tEBP = 0x%08x\n"
        "ESI = 0x%08x\tEDI = 0x%08x\n"
        "\n"
        "EIP = 0x%08x\tCS = 0x%04x\n"
        "EFLAGS = 0x%08x\n",
        registers->eax, registers->ecx,
        registers->edx, registers->ebx,
        registers->esp, registers->ebp,
        registers->esi, registers->edi,
        parameters->eip, (uint32_t)parameters->cs,
        parameters->eflags
    );
}

void idt_exception_no_status(uint32_t interrupt, struct interrupt_registers* registers, struct interrupt_parameters* parameters) {
    vga_printf("Hardware exception %u (%s)\n", interrupt, INTERRUPT_NAMES[interrupt]);
    idt_exception_dump_registers(registers, parameters);
    kernel_panic();
}

void idt_exception_status(uint32_t interrupt, struct interrupt_registers* registers, struct interrupt_parameters* parameters, uint32_t status) {
    vga_printf("Hardware exception %u (%s); status = %u\n", interrupt, INTERRUPT_NAMES[interrupt], status);
    idt_exception_dump_registers(registers, parameters);
    kernel_panic();
}

void idt_exceptions_load(void) {
    idt_make_interrupt_no_status(0, idt_exception_no_status, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT); //Divide by zero
    //TODO: Debug
    //TODO: Non-maskable interrupt
    idt_make_interrupt_no_status(3, idt_exception_no_status, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_3 | IDT_FLAG_PRESENT); //Breakpoint
    idt_make_interrupt_no_status(4, idt_exception_no_status, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT); //Overflow
    idt_make_interrupt_no_status(5, idt_exception_no_status, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT); //Bound Range Exceeded
    idt_make_interrupt_no_status(6, idt_exception_no_status, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT); //Invalid Opcode
    idt_make_interrupt_no_status(7, idt_exception_no_status, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT); //Device Not Availlable
    idt_make_interrupt_status(8, idt_exception_status, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT); //Double Fault
    idt_make_interrupt_no_status(9, idt_exception_no_status, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT); //Coprocessor Segment Overrun
    idt_make_interrupt_status(10, idt_exception_status, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT); //Invalid TSS
    idt_make_interrupt_status(11, idt_exception_status, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT); //Segment Not Present
    idt_make_interrupt_status(12, idt_exception_status, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT); //Stack-Segment Fault
    idt_make_interrupt_status(13, idt_exception_status, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT); //General Protection Fault
    idt_make_interrupt_status(14, idt_exception_status, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT); //Page Fault
    //TODO: Reserved
    idt_make_interrupt_no_status(16, idt_exception_no_status, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT); //x87 FPU Exception
    idt_make_interrupt_status(17, idt_exception_status, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT); //Alignment Check
    idt_make_interrupt_no_status(18, idt_exception_no_status, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT); //Machine Check
    //TODO: SIMD Floating-Point Exception
    //TODO: Virtualization Exception
    idt_make_interrupt_no_status(21, idt_exception_no_status, IDT_GATE_TYPE_TRAP_32, IDT_PRIVILEGE_0 | IDT_FLAG_PRESENT); //Reserved
}