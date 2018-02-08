#include "boot.h"

static inline void cpuid(u32 id, u32 * a, u32 * b, u32 * c, u32 * d) {
    asm volatile(
        "cpuid"
        : "=a" (*a), "=b" (*b), "=c" (*c), "=d" (*d)
        : "a" (id)
    );

}

struct cpu {
    char vendor[13];
};

void memcpy(void * dst, void * src, int size) {
    char * c_dst = (char *)dst;
    char * c_src = (char *)src;

    while(size--) {
      *c_dst++ = *c_src++;
    }
}

void boot_main() {
    u32 a, b, c, d;
    struct cpu cpu;

    io_clear();
    io_puts("yui-os v0.0.1\n");

    cpuid(0, &a, &b, &c, &d);

    memcpy(&cpu.vendor[0], &b, 4);
    memcpy(&cpu.vendor[4], &d, 4);
    memcpy(&cpu.vendor[8], &c, 4);
    cpu.vendor[12] = 0x00;

    //io_puts(cpu.vendor);
    //io_putc('\n');

    //io_putl(a);
}