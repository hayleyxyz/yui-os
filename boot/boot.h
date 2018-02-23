#pragma once

#include <types.h>

void console_clear();
void console_putc(char * ch);
void console_puts(const char * str);

#define CR0_PE     1 << 0
#define CR0_PG     1 << 31
#define CR4_PAE    1 << 5
#define CR4_PGE    1 << 7
#define EFER_LME   1 << 8
#define MSR_EFER    0xC0000080 // Used with rdmsr(MSR_EFER, ...)

#define PG_PRESENT  (1 << 0)
#define PG_RW       (1 << 1)
#define PG_USER     (1 << 2)
#define PG_PWT      (1 << 3)
#define PG_PCD      (1 << 4)
#define PG_ACCESSED (1 << 5)
#define PG_DIRTY    (1 << 6)
#define PG_PSE      (1 << 7)
#define PG_GLOBAL   (1 << 8)
#define PG_PSE_PAT  (1 << 12)
#define PG_PKRU     (1 << 59)
#define PG_NX       (1 << 63)

static inline void rdmsr(uint32_t msr, uint32_t * d, uint32_t * a) {
    asm volatile(
        "rdmsr"
        : "=d" (*d), "=a" (*a)
        : "c" (msr)
    );
}

static inline void wrmsr(uint32_t msr, uint32_t d, uint32_t a) {
    asm volatile(
        "wrmsr"
        :: "d" (d), "a" (a), "c" (msr)
    );
}

#define WRITE_CRX(x) \
    static inline void write_cr##x(uint32_t cr) { \
        asm volatile("mov %%eax, %%cr"#x :: "a" (cr)); \
    }

#define READ_CRX(x) \
    static inline uint32_t read_cr##x() { \
        uint32_t a; \
        asm volatile("mov %%cr"#x", %%eax" : "=a" (a)); \
        return a; \
    }

READ_CRX(0)
WRITE_CRX(0)
READ_CRX(4)
WRITE_CRX(4)
WRITE_CRX(3)

static inline void cpuid(uint32_t id, uint32_t * a, uint32_t * b, uint32_t * c, uint32_t * d) {
    asm volatile(
        "cpuid"
        : "=a" (*a), "=b" (*b), "=c" (*c), "=d" (*d)
        : "a" (id)
    );

}

static inline void halt() {
    for(;;) asm volatile("hlt");
}
