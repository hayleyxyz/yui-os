OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)

ENTRY(_start)

SECTIONS
{
    . = 0;

    .text ALIGN(4K) : {
        *(.multiboot)
        *(.text)
        *(.rodata)
        *(.data)
        *(.longmode)
    }

    .bss ALIGN(4K) : {
        *(.bss)
    }
}