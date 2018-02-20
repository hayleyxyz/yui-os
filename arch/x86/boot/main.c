#include "boot.h"
#include "multiboot.h"

extern const void pml4t, pdpt, pdt, pt;

#define NO_OF_PT_ENTRIES        512

#define PAGE_SIZE_2MB           0x200000

volatile uint64_t pml4[NO_OF_PT_ENTRIES] __attribute__((aligned(4096)));
volatile uint64_t pdp[NO_OF_PT_ENTRIES] __attribute__((aligned(4096))); /* temporary */
volatile uint64_t pte[NO_OF_PT_ENTRIES * NO_OF_PT_ENTRIES] __attribute__((aligned(4096)));

#if 0
static void enable_paging() {
    u32 i;

    // Populate paging tables
    // TODO: PML5
    *(u32*)&pml4t = (u32)&pdpt | PG_PRESENT | PG_RW;
    *(u32*)&pdpt = (u32)&pdt | PG_PRESENT | PG_RW;
    *(u32*)&pdt = (u32)&pt | PG_PRESENT | PG_RW;

    for(i = 0; i < 4096 / 8; i++) {
        ((u32*)&pt)[i * 2] = (i * 0x1000) | PG_PRESENT | PG_RW;
    }

    // Set cr3 to pml4t address
    write_cr3(read_cr3() | (u32)&pml4t);

    // Enable Physical Address Extension
    write_cr4(read_cr4() | CR4_PAE);
}
#endif

static void init_mapping() {
    pml4[0] = (uint32_t)&pdp[0] | PG_PRESENT | PG_RW;
    pdp[0] = (uint32_t)&pte[0] | PG_PRESENT | PG_RW;
}

static void map_page(uint64_t physical_addr, uint64_t virtual_addr) {
    uint32_t pd_index = (virtual_addr / PAGE_SIZE_2MB) / NO_OF_PT_ENTRIES;
    uint32_t pt_index = (pd_index * NO_OF_PT_ENTRIES) + ((virtual_addr / PAGE_SIZE_2MB) % NO_OF_PT_ENTRIES);

    if((pdp[pd_index] & PG_PRESENT) == 0) {
        pdp[pd_index] = (uint32_t)&pte[pt_index] | PG_PRESENT | PG_RW;
    }

    pte[pt_index] = physical_addr | PG_PRESENT | PG_RW | PG_PSE;

    io_iprintf("b 0x%08x\n", pt_index);
    io_iprintf("b 0x%08x\n", virtual_addr | PG_PRESENT | PG_RW | PG_PSE);
}

static void enable_paging() {
    init_mapping();

    // Identity map first 4MB
    map_page(0, 0);
    map_page(PAGE_SIZE_2MB, PAGE_SIZE_2MB);

    // Map physical 0x00 to 0xc0000000
    map_page(0, 0xc0000000);

    // Set cr3 to pml4t address
    write_cr3((uint32_t)&pml4[0]);

    // Enable Physical Address Extension
    write_cr4(read_cr4() | CR4_PAE);
}

static void enable_longmode() {
    u32 d, a;

    rdmsr(MSR_EFER, &d, &a);
    a |= EFER_LME;
    wrmsr(MSR_EFER, d, a);
}

static inline void halt() {
    asm volatile("cli\nhlt");
}

void multiboot_main(u32 mb_magic, struct multiboot_info * mb_info) {
    struct multiboot_mmap_entry * mmap_entry;
    int i = 0;

    io_clear();
    io_puts("yui-os v0.0.1\n");

    if(mb_magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        io_puts("Multiboot magic invalid!");
        halt();
    }

    if(mb_info->flags & MULTIBOOT_INFO_MEM_MAP == 0) {
        io_puts("No memory map provided by the bootloader!");
        halt();
    }


    io_iprintf("mmap_length: 0x%08x\n", mb_info->mmap_length);
    io_iprintf("mmap_addr: 0x%08x\n", mb_info->mmap_addr);

    mmap_entry = (struct multiboot_mmap_entry *)mb_info->mmap_addr;

    for(i = 0; i < mb_info->mmap_length / sizeof(struct multiboot_mmap_entry); i++) {
        io_iprintf("addr: 0x%08x len: 0x%08x type: 0x%08x - \n", mmap_entry[i].addr, mmap_entry[i].len, mmap_entry[i].type);
    }


    // TOOD: cpuid checks

    enable_paging();

    enable_longmode();

    // Enable paging in longmode
    write_cr0(read_cr0() | CR0_PG);

    io_iprintf("test: %s\n", (char*)0xc0000000);
    io_iprintf("test: 0x%x\n", *(uint32_t*)0xc0000000);

}