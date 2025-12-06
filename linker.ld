/* linker.ld – i686 kernel linker script */

ENTRY(_start)                     /* boot/boot.asm must define _start */

SECTIONS
{
    /* Kernel is loaded at 1 MiB (0x100000) – standard for higher-half or just safe physical address */
    . = 1M;    
    kernel_start = .;

    /* ============== .text – code (read+execute only) ============== */
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)         /* multiboot header must be in first 8K */
        *(.text .text.*)      /* all code sections */
    } =0x90909090             /* fill unused bytes with NOPs */

    /* ============== .rodata – read-only data (read only) ============== */
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata .rodata.*)
    }

    /* ============== .data – initialized data (read+write) ============== */
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data .data.*)
    }

    /* ============== .bss – uninitialized data (read+write) ============== */
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss .bss.*)
    }

    /* Discard useless sections */
    /DISCARD/ :
    {
        *(.eh_frame)
        *(.comment)
        *(.note*)
    }
    kernel_end   = .;
}