MULTIBOOT_PAGE_ALIGN    equ (1 << 0)
MULTIBOOT_MEMORY_INFO   equ (1 << 1)
MULTIBOOT_AOUT_KLUDGE   equ (1 << 16)
MULTIBOOT_HEADER_MAGIC  equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS  equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_AOUT_KLUDGE
MULTIBOOT_CHECKSUM      equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

STACK_SIZE              equ 4096

section .multiboot
align 4
_multiboot:
    dd MULTIBOOT_HEADER_MAGIC   ; magic
    dd MULTIBOOT_HEADER_FLAGS   ; flags
    dd MULTIBOOT_CHECKSUM       ; checksum
    dd 0                        ; header_addr
    dd _multiboot               ; load_addr
    dd 0                        ; load_end_addr
    dd _bss_end                 ; bss_end_addr
    dd _start                   ; entry_addr



[BITS 32]
extern multiboot_main
section .text
global _start
_start:
    mov     esp, stack_end

    push    ebx
    push    eax
    call    multiboot_main

    lgdt    [GDT64.Pointer]
    jmp     GDT64.Code:_longmode_start

    hlt

section .bss
align 4096
stack_start:
    resb STACK_SIZE
stack_end:

global pml4t, pdpt, pdt, pt
pml4t:
    resb 4096
pdpt:
    resb 4096
pdt:
    resb 4096
pt:
    resb 4096

_bss_end:

section .data
align 16
GDT64:                           ; Global Descriptor Table (64-bit).
    .Null: equ $ - GDT64         ; The null descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 0                         ; Access.
    db 0                         ; Granularity.
    db 0                         ; Base (high).
    .Code: equ $ - GDT64         ; The code descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 10011010b                 ; Access (exec/read).
    db 00100000b                 ; Granularity.
    db 0                         ; Base (high).
    .Data: equ $ - GDT64         ; The data descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 10010010b                 ; Access (read/write).
    db 00000000b                 ; Granularity.
    db 0                         ; Base (high).
    .Pointer:                    ; The GDT-pointer.
    dw $ - GDT64 - 1             ; Limit.
    dq GDT64                     ; Base.


[BITS 64]
section .longmode
;extern kernel_main
_longmode_start:
    cli                           ; Clear the interrupt flag.
    mov ax, GDT64.Data            ; Set the A-register to the data descriptor.
    mov ds, ax                    ; Set the data segment to the A-register.
    mov es, ax                    ; Set the extra segment to the A-register.
    mov fs, ax                    ; Set the F-segment to the A-register.
    mov gs, ax                    ; Set the G-segment to the A-register.
    mov ss, ax                    ; Set the stack segment to the A-register.
    mov edi, 0xB8000              ; Set the destination index to 0xB8000.
    mov rax, 0x1F201F201F201F20   ; Set the A-register to 0x1F201F201F201F20.
    mov ecx, 500                  ; Set the C-register to 500.
    rep stosq                     ; Clear the screen.
    ;call kernel_main
    hlt