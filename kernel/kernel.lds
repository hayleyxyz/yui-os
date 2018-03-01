ENTRY(kernel_main)

SECTIONS
{
    . = 0x00001000;

    .text ALIGN(4096) : {
        QUAD(kernel_main)
        *(.text)
        *(.rodata)
        *(.data)
        *(.got.plt)
        *(code)
        *(data)
        *(.data.rel.local)
    }
    
    .bss ALIGN(4096) : {
        *(.bss)
    }
}