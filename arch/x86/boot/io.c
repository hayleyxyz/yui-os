#include <types.h>

volatile u16 * base_text_buffer = (volatile u16 *)0xB8000;

volatile u16 * current_text_buffer = (volatile u16 *)0xB8000;

static const u16 TERM_WIDTH = 80;
static const u16 TERM_HEIGHT = 25;

static char internal_buffer[50];

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

void swap(int * a, int * b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void reverse(char str[], int length) {
    int start = 0;
    int end = length -1;

    while (start < end) {
        swap(*(str+start), *(str+end));
        start++;
        end--;
    }
}

// TODO: str length
void inttostr(u32 num, char * str, int base) {
    u32 i = 0;
    bool neg = false;

    if(num == 0) {
        str[i] = '0';
        str[++i] = 0x00;
    }

    if(num < 0 && base == 10) {
        neg = true;
        num = -num;
    }

    while(num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem-10) + 'a' : rem + '0';
        num = num / base;
    }

    if(neg) {
        str[i++] = '-';
    }

    str[i] = 0x00;

}

void io_putl(u32 num) {
    inttostr(num, &internal_buffer, 10);
    io_puts(num);
}