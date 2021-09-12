[BITS 32]

GLOBAL pt_invalidate_tlb
GLOBAL pt_invalidate_address
GLOBAL pt_load_directory

SECTION .text

pt_invalidate_tlb:
    mov eax, cr3
    mov cr3, eax
    ret

pt_invalidate_address:
    mov eax, [esp + 4]
    invlpg [eax]
    ret
