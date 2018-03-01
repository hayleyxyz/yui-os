#include <idt.h>
#include <console.h>
#include <cpu.h>

struct idt idt;

struct idtr idtr = {
    .limit = sizeof(idt) - 1,
    .address = (uintptr_t)&idt
};

extern uintptr_t const _interrupt_table[];

void idt_init() {
    uintptr_t offset;

    for(uint32_t i = 0; i < 256; i++) {
        offset = _interrupt_table[i];

        idt.gates[i].offset_low = offset;
        idt.gates[i].segment = 0x08; // GDT64.Code
        idt.gates[i].bits.ist = 0; // Interrupt Stack Table, not used
        idt.gates[i].bits.p = 1; // present
        idt.gates[i].bits.dpl = 0; // ring 0
        idt.gates[i].bits.zero = 0;
        idt.gates[i].bits.type = 0xE; // IDT_INTERRUPT_GATE64
        idt.gates[i].offset_middle = (uint16_t)(offset >> 16);
        idt.gates[i].offset_high = (uint16_t)(offset >> 32);
        idt.gates[i].reserved = 0;
    }

    lidt((uintptr_t)&idtr);
}

static const char * messages[] = {
    "Divide Error (#DE)",
    "Debug (#DB)",
    "NMI Interrupt",
    "Breakpoint (#BP)",
    "Overflow (#OF)",
    "BOUND Range Exceeded (#BR)",
    "Invalid Opcode (#UD)",
    "Device Not Available (#NM)",
    "Double Fault (#DF)",
    "Coprocessor Segment Over",
    "Invalid TSS (#TS)",
    "Segment Not Present (#NP)",
    "Stack Fault (#SS)",
    "General Protection (#GP)",
    "Page-Fault (#PF)",
    "Undefined",
    "x87 FPU Error (#MF)",
    "Alignment Check (#AC)",
    "Machine-Check (#MC)",
    "SIMD Floating-Point (#XM)",
};

void exception_handler(struct x86_64_iframe * frame) {
    const char * message;

    if(frame->vector <= 19) {
        message = messages[frame->vector];
    }
    else {
        message = "Unknown Interrupt";
    }

    printk("%d - %s 0x%x\nrip: 0x%016llx\n",
        frame->vector, message, frame->err_code, frame->ip);

    printk("rax: 0x%016llx rcx: 0x%016llx rdx: 0x%016llx\n"
        "rbx: 0x%016llx rbp: 0x%016llx rsi: 0x%016llx\n"
        "rdi: 0x%016llx",
        frame->rax, frame->rcx, frame->rdx, frame->rbx, frame->rbp, frame->rsi, frame->rdi);

    halt();
}

#if 0
struct x86_64_iframe {
    uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;         // pushed by common handler
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;      // pushed by common handler
    uint64_t vector;                                    // pushed by stub
    uint64_t err_code;                                  // pushed by interrupt or stub
    uint64_t ip, cs, flags;                             // pushed by interrupt
    uint64_t user_sp, user_ss;                          // pushed by interrupt
};
#endif