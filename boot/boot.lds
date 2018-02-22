OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)

ENTRY(_start)

SECTIONS
{
    . = 0;

    .text ALIGN(4096) : {
        *(.multiboot)
        *(.text)
        *(.rodata)
        *(.data)
        *(.longmode)
    }

     _text_end = ALIGN(4096);

    . = 0x10000;

    .bss ALIGN(4096) : {
        *(.bss)
    }

    _bss_end = .;
}