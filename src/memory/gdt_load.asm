GLOBAL gdt_load
SECTION .text

gdt_load:
    mov eax, [esp+4]
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