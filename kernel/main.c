#include <types.h>
#include <bootdata.h>
#include <console.h>
#include <cpu.h>
#include <memory.h>
#include <idt.h>
#include <memset.h>
#include <gdt.h>

volatile void * gdt_base;

#define IO_BITMAP_BITS      65536
#define IO_BITMAP_BYTES     (IO_BITMAP_BITS/8)

struct tss_x86_64 {
    uint32_t rsvd0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint32_t rsvd1;
    uint32_t rsvd2;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint32_t rsvd3;
    uint32_t rsvd4;
    uint16_t rsvd5;
    uint16_t iomap_base;

    uint8_t tss_bitmap[IO_BITMAP_BYTES + 1];
} __attribute__((packed));

typedef struct tss_x86_64 tss_64_t;

struct seg_desc_x86_64 {
    uint16_t limit_15_0;
    uint16_t base_15_0;
    uint8_t base_23_16;

    uint8_t type : 4;
    uint8_t s : 1;
    uint8_t dpl : 2;
    uint8_t p : 1;

    uint8_t limit_19_16 : 4;
    uint8_t avl : 1;
    uint8_t reserved_0 : 1;
    uint8_t d_b : 1;
    uint8_t g : 1;

    uint8_t base_31_24;

    uint32_t base_63_32;
    uint32_t reserved_sbz;
} __attribute__((packed));

void set_global_desc_64(struct seg_desc_x86_64* g, uint64_t base, uint32_t limit,
                        uint8_t present, uint8_t ring, uint8_t sys,
                        uint8_t type, uint8_t gran, uint8_t bits) {
    g->limit_15_0 = limit & 0x0000ffff;
    g->limit_19_16 = (limit & 0x000f0000) >> 16;

    g->base_15_0 = base & 0x0000ffff;
    g->base_23_16 = (base & 0x00ff0000) >> 16;
    g->base_31_24 = (base & 0xff000000) >> 24;
    g->base_63_32 = (uint32_t)(base >> 32);

    g->type = type & 0x0f; // segment type
    g->p = present != 0;   // present
    g->dpl = ring & 0x03;  // descriptor privilege level
    g->g = gran != 0;      // granularity
    g->s = sys != 0;       // system / non-system
    g->d_b = bits != 0;    // 16 / 32 bit

}

#define NUM_ASSIGNED_IST_ENTRIES 3
volatile uint8_t interrupt_stacks[NUM_ASSIGNED_IST_ENTRIES][4096] __attribute__((aligned(16)));

#if __GNUC__ > 3
#define offsetof(type, member) __builtin_offsetof(type, member)
#else
#define offsetof(type, member) ((size_t)((char*)&(((type*)0)->member) - (char*)0))
#endif

static inline void x86_ltr(uint16_t sel) {
    __asm__ __volatile__ ("ltr %%ax" :: "a" (sel));
}

extern struct memory_block * head_memory_block;

static inline void dump_blocks() {
    struct memory_block * block = head_memory_block;

    printk("\nDUMP MEM. BLOCKS\n");

    while(block) {
        printk("addr: 0x%08x%08x len: 0x%08x%08x available: %c\n",
                block->address >> 32, block->address & 0xffffffff, block->length >> 32,
                block->length & 0xffffffff, block->available ? 'y' : 'n');

        block = block->next;
    }
}

void kernel_main(struct bootdata * bootdata) {
    console_clear();
    console_puts("yui-os KERNEL v0.0.1\n");

    if(bootdata->magic != BOOTDATA_MAGIC) {
        console_puts("Bootdata magic invalid!");
        halt();
    }

    idt_init();

    init_memory(bootdata);
    

    struct tss_x86_64 tss;
    memset(&tss, 0, sizeof(tss));

    gdt_base = (void *)bootdata->gdt;

    struct seg_desc_x86_64 * pgdt = (struct seg_desc_x86_64 *)(bootdata->gdt + bootdata->gdt_tss);

    set_global_desc_64(pgdt, (uintptr_t)&tss, sizeof(struct tss_x86_64) - 1, 1, 0, 0, SEG_TYPE_TSS, 0, 0);

    tss.ist1 = (uintptr_t)&interrupt_stacks[0] + 4096;
    tss.ist2 = (uintptr_t)&interrupt_stacks[1] + 4096;
    tss.ist3 = (uintptr_t)&interrupt_stacks[2] + 4096;

    tss.iomap_base = offsetof(tss_64_t, tss_bitmap);

    tss.tss_bitmap[IO_BITMAP_BYTES] = 0xff;

    x86_ltr(GDT_TSS);


    #if 0
    uintptr_t base = (uintptr_t)&tss;
    uint64_t limit = sizeof(tss);
    uint32_t type = SEG_TYPE_TSS;

    struct gdt_x86_64 gdt = {
        .limit_15_0 = limit & 0x0000ffff,
        .limit_19_16 = (limit & 0x000f0000) >> 16,

        .base_15_0 = base & 0x0000ffff,
        .base_23_16 = (base & 0x00ff0000) >> 16,
        .base_31_24 = (base & 0xff000000) >> 24,
        .base_63_32 = (uint32_t)(base >> 32),

        .type = type & 0x0f; // segment type
        .p = present != 0;   // present
        .dpl = ring & 0x03;  // descriptor privilege level
        .g = gran != 0;      // granularity
        .s = sys != 0;       // system / non-system
        .d_b = bits != 0;    // 16 / 32 bit
    };
    #endif

    
}