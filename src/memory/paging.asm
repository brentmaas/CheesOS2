[BITS 32]

CR0_ENABLE_PAGING_BIT equ 1 << 31

GLOBAL paging_enable
GLOBAL paging_disable
GLOBAL paging_invalidate_tlb
GLOBAL paging_invalidate_address
GLOBAL paging_load_directory

SECTION .text

paging_enable:
    mov eax, cr0
    or eax, CR0_ENABLE_PAGING_BIT
    mov cr0, eax
    ret

paging_disable:
    mov eax, cr0
    and eax, ~CR0_ENABLE_PAGING_BIT
    mov cr0, eax
    ret

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
