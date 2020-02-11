[BITS 32]

EXTERN idt_callback_routines
GLOBAL idt_create_handler_table

SECTION .text

%macro interrupt_no_status 1
interrupt_handler_%1:
    ; Save registers
    pusha

    ; Fetch syscall parameters
    lea eax, [esp+104]
    ; Fetch registers
    mov edx, esp

    ; Push syscall parameters as argument
    push edx
    ; Push registers
    push eax

    ; Call handler
    call [idt_callback_routines+4*%1]

    ; Restore stack and registers
    add esp, 8
    popa

    ; Return from interrupt handler
    iret

%endmacro

%macro interrupt_status 1
interrupt_handler_%1:
    ; Save registers
    pusha

    ; Fetch syscall parameters
    lea eax, [esp+108]
    ; Fetch registers
    mov edx, esp
    ; Fetch status code
    mov ecx, [esp+72]

    ; Push status code
    push ecx
    ; Push syscall parameters as argument
    push eax
    ; Push registers
    push edx

    ; Call handler
    call [idt_callback_routines+4*%1]

    ; Restore stack and registers
    add esp, 12
    popa

    ; Return from interrupt handler
    iret
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