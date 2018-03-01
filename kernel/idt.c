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

    printk("%d - %s 0x%x\nRIP: 0x%016llx\n", frame->vector, message, frame->err_code, frame->ip);

    halt();
}