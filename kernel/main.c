#include <types.h>
#include <bootdata.h>
#include <console.h>
#include <cpu.h>
#include <memory.h>
#include <idt.h>


void kernel_main(struct bootdata * bootdata) {
    console_clear();
    console_puts("yui-os KERNEL v0.0.1\n");

    if(bootdata->magic != BOOTDATA_MAGIC) {
        console_puts("Bootdata magic invalid!");
        halt();
    }

    init_memory(bootdata);

    idt_init();

    __asm volatile("int $30");
    __asm volatile("int $31");

    printk("before div by zero\n");

//    volatile int x = 0/0;

    printk("after div by zero\n");

    for(;;) {}
}