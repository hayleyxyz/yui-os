#include <types.h>
#include <bootdata.h>
#include <console.h>
#include <cpu.h>
#include <memory.h>

struct idt_bits {
    uint16_t     ist : 3,
            zero    : 5,
            type    : 5,
            dpl : 2,
            p   : 1;
} __attribute__((packed));

struct idt_gate {
    uint16_t     offset_low;
    uint16_t     segment;
    struct idt_bits bits;
    uint16_t     offset_middle;
    uint32_t     offset_high;
    uint32_t     reserved;
} __attribute__((packed));

struct idt {
    struct idt_gate gates[256];
};

struct idtr {
    uint16_t limit;
    uintptr_t address;
} __attribute__((packed));

struct idt idt;

struct idtr idtr = {
    .limit = sizeof(idt) - 1,
    .address = (uintptr_t)&idt
};

static inline void lidt(uintptr_t base) {
    __asm volatile("lidt (%0)" :: "r"(base) : "memory");
}

void _irq_handler(void * s) {
    printk("_irq_handler\n");

    halt();
}

void kernel_main(struct bootdata * bootdata) {
    console_clear();
    console_puts("yui-os KERNEL v0.0.1\n");

    if(bootdata->magic != BOOTDATA_MAGIC) {
        console_puts("Bootdata magic invalid!");
        halt();
    }

    init_memory(bootdata);

    uintptr_t offset = (uintptr_t)_irq_handler;

    for(uint32_t i = 0; i < 256; i++) {
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

    // uintptr_t addr = alloc_page();
    // printk("alloc_page: 0x%08x%08x\n", addr >> 32, addr);
    // addr = alloc_page();
    // printk("alloc_page: 0x%08x%08x\n", addr >> 32, addr);

    printk("before div by zero\n");

    volatile int x = 0/0;

    printk("after div by zero\n");

    __asm volatile("int $0x30");

    *(uint64_t*)0xfe1000000 = 0x00;
}