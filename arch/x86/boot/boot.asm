MULTIBOOT_PAGE_ALIGN    equ (1 << 0)
MULTIBOOT_MEMORY_INFO   equ (1 << 1)
MULTIBOOT_HEADER_MAGIC  equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS  equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
MULTIBOOT_CHECKSUM      equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

STACK_SIZE              equ 4096

section .multiboot
align 4
    dd MULTIBOOT_HEADER_MAGIC
    dd MULTIBOOT_HEADER_FLAGS
    dd MULTIBOOT_CHECKSUM
    dd 0, 0, 0, 0, 0 ; address fields

[BITS 32]
extern boot_main
section .text
global _start
_start:
    mov     esp, stack_end

    push    ebx
    push    eax
    call    boot_main

    cli
l:  hlt
    jmp     l

section .bss
align 4096
stack_start:
    resb STACK_SIZE
stack_end:

global _pml4t, _pdpt, _pdt, _pt
_pml4t:
    resb 4096
_pdpt:
    resb 4096
_pdt:
    resb 4096
_pt:
    resb 4096