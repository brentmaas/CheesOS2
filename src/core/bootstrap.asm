[BITS 32]

CR0_ENABLE_PAGING_BIT equ 1 << 31

EXTERN vmm_bootstrap
EXTERN multiboot_bootstrap

EXTERN kernel_main
EXTERN kernel_panic
EXTERN kernel_page_dir

EXTERN kernel_virtual_start

EXTERN _init
EXTERN _fini

GLOBAL _start

SECTION .bss
ALIGN 16
kernel_stack_bottom:
RESB 16*1024
kernel_stack_top:

SECTION .bootstrap.text
_start:
    mov esp, kernel_stack_top
    sub esp, kernel_virtual_start ; For some reason this can't be done at compiletime

    call vmm_bootstrap
    mov cr3, eax

    push ebx
    call multiboot_bootstrap
    add esp, 4
    mov ebx, eax

    mov eax, cr0
    or eax, CR0_ENABLE_PAGING_BIT
    mov cr0, eax
    jmp kernel_bootstrap

SECTION .text
kernel_bootstrap:
    mov esp, kernel_stack_top

    call _init

    push ebx
    call kernel_main
    pop ebx

    call _fini

    call kernel_panic
