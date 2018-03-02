MULTIBOOT_PAGE_ALIGN    equ (1 << 0)
MULTIBOOT_MEMORY_INFO   equ (1 << 1)
MULTIBOOT_AOUT_KLUDGE   equ (1 << 16)
MULTIBOOT_HEADER_MAGIC  equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS  equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_AOUT_KLUDGE
MULTIBOOT_CHECKSUM      equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

STACK_SIZE              equ 4096

extern multiboot_main, _bss_end, bootdata_ptr, stack

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
section .text
global _start
_start:
    mov     edx, stack
    add     edx, 4096
    mov     esp, edx

    push    ebx ; mb_info
    push    eax ; mb magic
    call    multiboot_main

    lgdt    [GDT64.Pointer]
    jmp     GDT64.Code:_longmode_start
    ud2

global GDT64, GDT64_TSS
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
    GDT64_Code: equ GDT64.Code
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 0b10011010                ; P(1) DPL(00) S(1) 1 C(0) R(1) A(0)
    db 0b10101111                ; G(1) D(0) L(1) AVL(0) limit 19:16
    db 0                         ; Base (high).
    .Data: equ $ - GDT64         ; The data descriptor.
    GDT64_Data: equ GDT64.Data
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 0b10010010                ; P(1) DPL(00) S(1) 0 E(0) W(1) A(0)
    db 0b11001111                ; G(1) B(1) 0 0 limit 19:16
    db 0                         ; Base (high).
    .UserCode: equ $ - GDT64     ; The code descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 0b11111010                ; P(1) DPL(11) S(1) 1 C(0) R(1) A(0)
    db 0b10101111                ; G(1) D(0) L(1) AVL(0) limit 19:16
    db 0                         ; Base (high).
    .UserData: equ $ - GDT64     ; The data descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 0b11110010                ; P(1) DPL(11) S(1) 0 E(0) W(1) A(0)
    db 0b11001111                ; G(1) B(1) 0 0 limit 19:16
    db 0                         ; Base (high).
    .TSS: equ $ - GDT64
    GDT64_TSS: equ GDT64.TSS
    dw 0                ; limit 15:00
    dw 0                ; base 15:00
    db  0                ; base 23:16
    db  0b10001001       ; P(1) DPL(00) 0 10 B(0) 1
    db  0b10000000       ; G(1) 0 0 AVL(0) limit 19:16
    db  0                ; base 31:24
    ; second half of 64bit desciptor
    dd   0x00000000       ; base 63:32
    dd   0x00000000       ; reserved/sbz
    .Pointer:                    ; The GDT-pointer.
    dw $ - GDT64 - 1             ; Limit.
    dq GDT64                     ; Base.

[BITS 64]
section .longmode
_longmode_start:
    cli
    mov ax, GDT64.Data
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov rdx, 0x1000
    mov rdx, [rdx]
    mov rdi, [bootdata_ptr]
    call rdx

    hlt