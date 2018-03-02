#pragma once

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