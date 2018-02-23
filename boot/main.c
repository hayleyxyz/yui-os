#include "boot.h"
#include "multiboot.h"
#include <bootdata.h>

#define NO_OF_PT_ENTRIES        512

#define PAGE_SIZE_2MB           0x200000

volatile uint64_t pml4[NO_OF_PT_ENTRIES] __attribute__((aligned(4096)));
volatile uint64_t pdp[NO_OF_PT_ENTRIES] __attribute__((aligned(4096)));
volatile uint64_t pte[NO_OF_PT_ENTRIES * NO_OF_PT_ENTRIES] __attribute__((aligned(4096)));

volatile struct bootdata bootdata;

#define MMAP_ENTRIES 24
volatile struct mmap mmap[MMAP_ENTRIES];

volatile uintptr64_t bootdata_ptr;

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
}

static void enable_paging() {
    init_mapping();

    // Identity map first 4MB
    map_page(0, 0);
    map_page(PAGE_SIZE_2MB, PAGE_SIZE_2MB);

    // Map physical 0x00 to 0xc0000000
    //map_page(0, 0xc0000000);

    // Set cr3 to pml4 address
    write_cr3((uint32_t)&pml4[0]);

    // Enable Physical Address Extension
    write_cr4(read_cr4() | CR4_PAE);
}

static void enable_longmode() {
    uint32_t d, a;

    rdmsr(MSR_EFER, &d, &a);
    a |= EFER_LME;
    wrmsr(MSR_EFER, d, a);
}

static void setup_bootdata(struct multiboot_info * mb_info) {
    uint32_t i = 0;
    struct multiboot_mmap_entry * mmap_entry = (struct multiboot_mmap_entry *)mb_info->mmap_addr;

    bootdata.mmap_count = 0;

    for(i = 0; i < mb_info->mmap_length / sizeof(struct multiboot_mmap_entry); i++) {
        mmap[i].addr = mmap_entry[i].addr;
        mmap[i].len = mmap_entry[i].len;
        mmap[i].type = mmap_entry[i].type;
        bootdata.mmap_count++;
    }

    bootdata.mmap = EXTEND_POINTER(&mmap);
    bootdata.magic = BOOTDATA_MAGIC;
    bootdata.pml4 = EXTEND_POINTER(&pml4);
    bootdata.pdp = EXTEND_POINTER(&pdp);
    bootdata.pte = EXTEND_POINTER(&pte);

    bootdata_ptr = (uintptr64_t)((uintptr_t)&bootdata);
}

/**
 * TODO: remap to 0x00000500 or 0x00007E00
 */

void multiboot_main(uint32_t mb_magic, struct multiboot_info * mb_info) {
    console_clear();
    console_puts("yui-os BOOT v0.0.1\n");

    if(mb_magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        console_puts("Multiboot magic invalid!");
        halt();
    }

    if((mb_info->flags & MULTIBOOT_INFO_MEM_MAP) == 0) {
        console_puts("No memory map provided by the bootloader!");
        halt();
    }

    setup_bootdata(mb_info);

    enable_paging();

    enable_longmode();

    // Enable paging in longmode
    write_cr0(read_cr0() | CR0_PG);

    // Return to boot code
    return;
}