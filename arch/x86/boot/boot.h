#ifndef _BOOT_H
#define _BOOT_H

#include <types.h>

void io_clear();
void io_putc(char * ch);
void io_puts(const char * str);
void io_putl(u32 num);

#endif