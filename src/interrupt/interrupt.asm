[BITS 32]

EXTERN idt_callback_routines
GLOBAL idt_load
GLOBAL idt_enable
GLOBAL idt_disable
GLOBAL idt_create_handler_table

SECTION .text

REGISTERS_STRUCT_SIZE equ 12 * 4

idt_load:
    mov eax, [esp+4]
    lidt [eax]
    ret

idt_enable:
    sti
    ret

idt_disable:
    cli
    ret

interrupt_handler_no_status:
    ; Save data segments
    ; cs and ss are handled by the cpu
    push gs
    push fs
    push es
    push ds

    ; save registers
    pusha

    ; Restore kernel data segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Fetch interrupt parameters
    lea edx, [esp + REGISTERS_STRUCT_SIZE + 4]
    ; Fetch interrupt number
    mov ecx, [esp + REGISTERS_STRUCT_SIZE]
    ; Fetch registers
    mov eax, esp

    ; Push handler parameters: interrupt number, registers,
    ; interrupt parameters (in reverse order)
    push edx
    push eax
    push ecx

    call [idt_callback_routines + 4 * ecx]

    ; Restore stack
    add esp, 12

    ; Restore registers
    popa

    ; Restore segments
    pop ds
    pop es
    pop fs
    pop gs

    ; Pop interrupt number
    add esp, 4

    ; Return from interrupt handler
    iret

interrupt_handler_status:
    ; Save data segments
    ; cs and ss are handled by the cpu
    push gs
    push fs
    push es
    push ds

    ; save registers
    pusha

    ; Restore kernel data segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Fetch interrupt parameters
    lea edx, [esp + REGISTERS_STRUCT_SIZE + 4 + 4]
    ; Fetch status code
    mov esi, [esp + REGISTERS_STRUCT_SIZE + 4]
    ; Fetch interrupt number
    mov ecx, [esp + REGISTERS_STRUCT_SIZE]
    ; Fetch registers
    mov eax, esp

    ; Push handler parameters: interrupt number, registers,
    ; interrupt parameters, status code (in reverse order)
    push esi
    push edx
    push eax
    push ecx

    call [idt_callback_routines + 4 * ecx]

    ; Restore stack
    add esp, 16

    ; Restore registers
    popa

    ; Restore segments
    pop ds
    pop es
    pop fs
    pop gs

    ; Pop interrupt number and error code
    add esp, 8

    ; Return from interrupt handler
    iret

%macro interrupt_no_status 1
interrupt_handler_%1:
    push %1
    jmp interrupt_handler_no_status
%endmacro

%macro interrupt_status 1
interrupt_handler_%1:
    push %1
    jmp interrupt_handler_status
%endmacro

%macro load_interrupt_location 1
    mov eax, interrupt_handler_%1
%endmacro

idt_create_handler_table:
    mov ecx, [esp+4]
    %assign i 0
    %rep 256
        load_interrupt_location i
        mov [ecx+4*i], eax
    %assign i i+1
    %endrep
    ret

; Interrupt handlers
interrupt_no_status 0
interrupt_no_status 1
interrupt_no_status 2
interrupt_no_status 3
interrupt_no_status 4
interrupt_no_status 5
interrupt_no_status 6
interrupt_no_status 7
interrupt_status 8
interrupt_no_status 9
interrupt_status 10
interrupt_status 11
interrupt_status 12
interrupt_status 13
interrupt_status 14
interrupt_no_status 15
interrupt_no_status 16
interrupt_status 17
interrupt_no_status 18
interrupt_no_status 19
interrupt_no_status 20
interrupt_no_status 21
interrupt_no_status 22
interrupt_no_status 23
interrupt_no_status 24
interrupt_no_status 25
interrupt_no_status 26
interrupt_no_status 27
interrupt_no_status 28
interrupt_no_status 29
interrupt_status 30
interrupt_no_status 31

%assign i 32
%rep 256-32
    interrupt_no_status i
    %assign i i+1
%endrep