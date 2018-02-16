#include <types.h>

volatile u16 * base_text_buffer_l64 = (volatile u16 *)0xB8000;

volatile u16 * current_text_buffer_l64 = (volatile u16 *)0xB8000;

static const u16 TERM_WIDTH = 80;
static const u16 TERM_HEIGHT = 25;

static inline u16 terminal_entry(char ch, u8 fg, u8 bg) {
    return (u16)ch | ((fg | bg << 4) << 8);
}

void io_putc_l64(char ch) {
    if(ch == '\n') {
        // Go to beginning of next line
        // TODO: clean this up
        current_text_buffer_l64 = (volatile u16 *)((u32)current_text_buffer_l64 +
            ((TERM_WIDTH * sizeof(u16)) - (((u32)current_text_buffer_l64 - (u32)base_text_buffer_l64) % TERM_WIDTH)));
    }
    else {
        *current_text_buffer_l64++ = terminal_entry(ch, 7, 0);
    }
}

void io_puts_l64(const char * str) {
    while(*str) {
        io_putc_l64(*str);
        str++;
    }
}

void kernel_main() {
	io_puts_l64("penisbutt");
}