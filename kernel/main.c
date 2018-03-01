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

    int x = 0/0;

    for(;;) {}
}