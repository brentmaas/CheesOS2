[BITS 64]

; Exports
global paging_disable

section .text
paging_disable:
    ; Fetch the old control register value
    mov rax, cr0

    ; Clear the paging bit
    and rax, 0x7FFFFFFF
    mov cr0, rax
    
    ret