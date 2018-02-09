#include <types.h>
#include <stdarg.h>
#include "mini-printf.h"

volatile u16 * base_text_buffer = (volatile u16 *)0xB8000;

volatile u16 * current_text_buffer = (volatile u16 *)0xB8000;

static const u16 TERM_WIDTH = 80;
static const u16 TERM_HEIGHT = 25;

static u8 internal_buffer[50];

static inline u16 terminal_entry(char ch, u8 fg, u8 bg) {
    return (u16)ch | ((fg | bg << 4) << 8);
}


void io_putc(char ch) {
    if(ch == '\n') {
        // Go to beginning of next line
        // TODO: clean this up
        current_text_buffer = (volatile u16 *)((u32)current_text_buffer +
            ((TERM_WIDTH * sizeof(u16)) - (((u32)current_text_buffer - (u32)base_text_buffer) % TERM_WIDTH)));
    }
    else {
        *current_text_buffer++ = terminal_entry(ch, 7, 0);
    }
}

void io_puts(const char * str) {
    while(*str) {
        io_putc(*str);
        str++;
    }
}

void io_clear() {
    int x, y;
    volatile u16 * copy;

    copy = current_text_buffer = base_text_buffer;

    for(y = 0; y < TERM_HEIGHT; y++) {
        for(x = 0; x < TERM_WIDTH; x++) {
            *copy++ = 0;
        }
    }
}

u32 strlen(u8 * str) {
    u32 i = 0;

    while(*str++) {
        i++;
    }

    return i;
}

u8 * strrev(u8 *str) {
    u8 *p1, *p2;

    if (! str || ! *str) {
        return str;
    }

    for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2) {
        *p1 ^= *p2;
        *p2 ^= *p1;
        *p1 ^= *p2;
    }
    return str;
}

u32 inttostr(u32 num, u8 * str, u32 len, u32 base)
{
    u32 sum = num;
    u32 i = 0;
    u32 digit;
    if (len == 0)
        return -1;
    do
    {
        digit = sum % base;
        if (digit < 0xA)
            str[i++] = '0' + digit;
        else
            str[i++] = 'A' + digit - 0xA;
        sum /= base;
    }while (sum && (i < (len - 1)));
    if (i == (len - 1) && sum)
        return -1;
    str[i] = '\0';
    strrev(str);
    return 0;
}

void io_putl(u32 num, u32 base) {
    inttostr(num, internal_buffer, sizeof(internal_buffer), base);
    io_puts((const char *)internal_buffer);
}

int io_iprintf(const char *fmt, ...)
{
    int ret;
    va_list va;
    va_start(va, fmt);
    ret = mini_vsnprintf(internal_buffer, sizeof(internal_buffer), fmt, va);
    va_end(va);

    io_puts(internal_buffer);

    return ret;
}