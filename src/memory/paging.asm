[BITS 32]

GLOBAL paging_invalidate_tlb
GLOBAL paging_invalidate_address
GLOBAL paging_load_directory

SECTION .text

paging_invalidate_tlb:
    mov eax, cr3
    mov cr3, eax
    ret

paging_invalidate_address:
    mov eax, [esp + 4]
    invlpg [eax]
    ret

paging_load_directory:
    mov eax, [esp + 4]
    mov cr3, eax
    ret
