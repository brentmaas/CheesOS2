ENTRY(_start)

SECTIONS
{
    kernel_virtual_start = 0xC0000000;

    . = 1M;
    kernel_physical_start = .;

    .bootstrap.text : ALIGN(4K)
    {
        *(.multiboot)
        *(.bootstrap.text)
    }

    . += kernel_virtual_start;

    .text : AT(ADDR(.text) - kernel_virtual_start) ALIGN(4K)
    {
        *(.text)

        *(.init.pre)
        *(.init)
        *(.init.post)

        *(.fini.pre)
        *(.fini)
        *(.fini.post)
    }

    .rodata : AT(ADDR(.rodata) - kernel_virtual_start) ALIGN(4K)
    {
        *(.rodata)
    }

    .data : AT(ADDR(.data) - kernel_virtual_start) ALIGN(4K)
    {
        *(.data)
    }

    .bss : AT(ADDR(.bss) - kernel_virtual_start) ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }

    kernel_virtual_end = .;
    kernel_physical_end = . - kernel_virtual_start;
}

/* Assert that we only require a single page table for all memory the kernel might require*/
ASSERT(kernel_physical_end < 4M, "Kernel does not fit into 4 MiB");
