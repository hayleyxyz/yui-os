#pragma once

#define CR0_PE     1 << 0
#define CR0_PG     1 << 31
#define CR4_PAE    1 << 5
#define CR4_PGE    1 << 7
#define EFER_LME   1 << 8
#define MSR_EFER    0xC0000080 // Used with rdmsr(MSR_EFER, ...)

#ifndef __ASSEMBLER__

static inline void halt() {
    for(;;) asm volatile("hlt");
}

static inline uint64_t rdtsc() {
    uint64_t tsc;
    uint32_t tsc_low;
    uint32_t tsc_hi;

    asm volatile("rdtsc" : "=a" (tsc_low), "=d" (tsc_hi));

    tsc = ((uint64_t)tsc_hi << 32) | tsc_low;

     return tsc;
}

#endif
