#include "boot.h"

extern volatile u32 _pml4t, _pdpt, _pdt, _pt;

#define DUMP_CR(x) \
    io_puts("cr"#x": 0x");  \
    io_putl(0, 10);

void dump_regs() {
    u32 a, d;

    io_iprintf("cr0: 0x%08x\n", read_cr0());
    io_iprintf("cr3: 0x%08x\n", read_cr3());
    io_iprintf("cr4: 0x%08x\n", read_cr4());

    rdmsr(MSR_EFER, &d, &a);
    io_iprintf("msr[efer]: edx: 0x%08x eax: 0x%08x", d, a);
    io_putc('-');
    io_putc('\n');
}

void multiboot_main() {
    u32 a, b, c, d, i;
    u32 * pt_ptr, ** pptr;

    io_clear();
    io_puts("yui-os v0.0.1\n");

    dump_regs();

    io_iprintf("_pml4t: 0x%08x\n", &_pml4t);
    io_iprintf("_pdpt: 0x%08x\n", &_pdpt);
    io_iprintf("_pdt: 0x%08x\n", &_pdt);
    io_iprintf("_pt: 0x%08x\n", &_pt);


    _pml4t = (u32)&_pdpt | 0b11;
    _pdpt = (u32)&_pdt | 0b11;
    _pdt = (u32)&_pt | 0b11;

    for(i = 0; i < 4096 / 8; i++) {
        pt_ptr = _pt;

        ((u32*)&_pt)[i * 2] = (i * 0x1000) | 0b11;
    }

    write_cr3(&_pml4t);

    rdmsr(MSR_EFER, &d, &a);
    a |= EFER_LME;
    wrmsr(MSR_EFER, d, a);

    write_cr4(read_cr4() | CR4_PAE);

    dump_regs();

    write_cr0(read_cr0() | CR0_PG | CR0_PE);
    
    io_puts("now in compatability mode?\n");
}