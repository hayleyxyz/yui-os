#pragma once

#include <types.h>

void console_clear();
void console_putc(char * ch);
void console_puts(const char * str);
int console_printf(const char *fmt, ...);

#define printk console_printf