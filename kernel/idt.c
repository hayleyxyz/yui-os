#include <idt.h>

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

#include <console.h>

void exception_handler(struct x86_64_iframe * frame) {
    printk("v: %d\n", frame->vector);
    printk("err_code: %d\n", frame->err_code);
    printk("rip: %d\n", frame->ip);
}