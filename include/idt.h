#pragma once

#include <types.h>

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

static inline void lidt(uintptr_t base) {
    __asm volatile("lidt (%0)" :: "r"(base) : "memory");
}

struct x86_64_iframe {
    uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;         // pushed by common handler
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;      // pushed by common handler
    uint64_t vector;                                    // pushed by stub
    uint64_t err_code;                                  // pushed by interrupt or stub
    uint64_t ip, cs, flags;                             // pushed by interrupt
    uint64_t user_sp, user_ss;                          // pushed by interrupt
};

void idt_init();