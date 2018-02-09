#ifndef _BOOT_H
#define _BOOT_H

#include <types.h>

void io_clear();
void io_putc(char * ch);
void io_puts(const char * str);
void io_putl(s32 num, u32 base);
int io_iprintf(const char *fmt, ...);

#define CR0_PE     1 << 0
#define CR0_PG     1 << 31
#define CR4_PAE    1 << 5
#define CR4_PGE    1 << 7
#define EFER_LME   1 << 8
#define MSR_EFER    0xC0000080 // Used with rdmsr(MSR_EFER, ...)

#define PG_PRESENT_MASK (1 << 0)

static inline void rdmsr(u32 msr, u32 * d, u32 * a) {
    asm volatile(
        "rdmsr"
        : "=d" (*d), "=a" (*a)
        : "c" (msr)
    );
}

static inline void wrmsr(u32 msr, u32 d, u32 a) {
    asm volatile(
        "wrmsr"
        :: "d" (d), "a" (a), "c" (msr)
    );
}

static inline u32 read_cr0() {
    u32 a;
    asm volatile("mov %%cr0, %%eax" : "=a" (a));
    return a;
}

static inline u32 read_cr3() {
    u32 a;
    asm volatile("mov %%cr3, %%eax" : "=a" (a));
    return a;
}

static inline u32 read_cr4() {
    u32 a;
    asm volatile("mov %%cr4, %%eax" : "=a" (a));
    return a;
}

static inline void write_cr0(u32 cr0) {
    asm volatile("mov %%eax, %%cr0" :: "a" (cr0));
}

static inline void write_cr3(u32 cr3) {
    asm volatile("mov %%eax, %%cr3" :: "a" (cr3));
}

static inline void write_cr4(u32 cr4) {
    asm volatile("mov %%eax, %%cr4" :: "a" (cr4));
}

static inline void cpuid(u32 id, u32 * a, u32 * b, u32 * c, u32 * d) {
    asm volatile(
        "cpuid"
        : "=a" (*a), "=b" (*b), "=c" (*c), "=d" (*d)
        : "a" (id)
    );

}

#endif