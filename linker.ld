/* KaiOS Linker Script */
/* Defines memory layout for the kernel */

ENTRY(_start)

SECTIONS
{
    /* Start at 1 MB (after conventional memory and BIOS) */
    . = 1M;

    /* Multiboot header must come first */
    .multiboot BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)
    }

    /* Text section (code) */
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.text)
    }

    /* Read-only data */
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    /* Read-write data (initialized) */
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    /* Read-write data (uninitialized) and stack */
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }

    /* End of kernel */
    _kernel_end = .;
}
