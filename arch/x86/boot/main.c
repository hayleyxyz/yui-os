#include "boot.h"

extern u32 pml4t, pdpt, pdt, pt;

extern u8 * _kernel64;

static void enable_paging() {
    u32 i;

    // Populate paging tables
    // TODO: PML5
    pml4t = (u32)&pdpt | PG_PRESENT | PG_RW;
    pdpt = (u32)&pdt | PG_PRESENT | PG_RW;
    pdt = (u32)&pt | PG_PRESENT | PG_RW;

    for(i = 0; i < 4096 / 8; i++) {
        ((u32*)&pt)[i * 2] = (i * 0x1000) | PG_PRESENT | PG_RW;
    }

    // Set cr3 to pml4t address
    write_cr3(read_cr3() | (u32)&pml4t);

    // Enable Physical Address Extension
    write_cr4(read_cr4() | CR4_PAE);
}

static void enable_longmode() {
    u32 d, a;

    rdmsr(MSR_EFER, &d, &a);
    a |= EFER_LME;
    wrmsr(MSR_EFER, d, a);
}

void multiboot_main() {
    io_clear();
    io_puts("yui-os v0.0.1\n");

    // TOOD: cpuid checks

    enable_paging();

    enable_longmode();

    // Enable paging in longmode
    write_cr0(read_cr0() | CR0_PG);


    //io_iprintf("_kernel64: %s\n", _kernel64);

    while(true) {

    }
}