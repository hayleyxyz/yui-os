extern exception_handler

[BITS 64]
segment code

GDT_USER_CODE equ ((8 * 3) | 3)
GDT_USER_DATA equ ((8 * 4) | 3)

global x86_uspace_entry
x86_uspace_entry:
    push qword GDT_USER_DATA ; ss
    push rdx                   ; sp
    push r8                    ; rflags
    push qword GDT_USER_CODE ; cs
    push rcx                   ; pc

    ; TODO: clear registers

    iretq



%macro _interrupt_stub_macro 1
global _interrupt_stub%1
_interrupt_stub%1:
    %if %1 == 8 || (%1 >= 10 && %1 <= 14) || %1 == 17
        push %1
        jmp _interrupt_handler_common
    %else
        push 0
        push %1
        jmp _interrupt_handler_common
    %endif
%endmacro

%assign i 0 
%rep 256 
    _interrupt_stub_macro i
%assign i i+1 
%endrep

_interrupt_handler_common:
    cld

    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    
    mov rdi, rsp

    call exception_handler

    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15

    ; drop vector number and error code
    add rsp, 16

    iretq


segment data

global _interrupt_table
_interrupt_table:
align 8
%assign i 0 
%rep 256 
    dq _interrupt_stub %+ i
%assign i i+1 
%endrep