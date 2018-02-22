#include <types.h>
#include <bootdata.h>
#include <console.h>

void kernel_main(struct bootdata * bootdata) {
    char buf[120];
    int i;

    console_clear();
    console_puts("yui-os KERNEL v0.0.1\n");

    console_printf("Hello World! 0x%08x", bootdata->mmap);

    for(i = 0; i < bootdata->mmap_count; i++) {
        console_printf("addr: 0x%08x%08x len: 0x%08x%08x type: 0x%08x\n",
            bootdata->mmap[i].addr >> 32,
            bootdata->mmap[i].addr & 0xffffffff,
            bootdata->mmap[i].len >> 32,
            bootdata->mmap[i].len & 0xffffffff,
            bootdata->mmap[i].type);
    }

    for(;;) { }
}