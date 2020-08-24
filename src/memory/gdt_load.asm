[BITS 32]

GLOBAL gdt_load
GLOBAL gdt_load_task_register
GLOBAL gdt_jump_to_usermode

SECTION .text

gdt_load:
    mov eax, [esp + 4]
    mov dx, 0x10

    lgdt [eax]

    jmp 0x8:gdt_load_cs_reload

gdt_load_cs_reload:
    mov ds, dx
    mov es, dx
    mov fs, dx
    mov gs, dx
    mov ss, dx
    ret

gdt_load_task_register:
    ltr [esp + 4]
    ret

gdt_jump_to_usermode:
    ; iret to another privilege level works as follows
    ; - Pop return instruction pointer
    ; - Pop code segment selector
    ; - Pop EFLAGS
    ; - Pop stack pointer
    ; - Pop stack selector

    mov ecx, [esp + 8] ; Usermode stack pointer
    mov edx, [esp + 4] ; Usermode code pointer

    ; Set usermode segments
    mov ax, 0x20 ; Data segment user level
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    or ax, 0x03 ; Set privilege level to 3 (user mode)
    push ax ; Stack segment
    push ecx
    pushf
    push 0x18 | 0x03 ; User code segment, privilege level 3
    push edx

    iret
