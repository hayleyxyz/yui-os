#pragma once

// https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html
// Chaper 3.5

#define SEG_TYPE_FLAG_DATA 0
#define SEG_TYPE_FLAG_EXPAND (1 << 2)
#define SEG_TYPE_FLAG_WRITE (1 << 1)
#define SEG_TYPE_FLAG_ACCESSED 1

#define SEG_TYPE_FLAG_CODE (1 << 3)
#define SEG_TYPE_FLAG_CONFORMING (1 << 2)
#define SEG_TYPE_FLAG_READ (1 << 1)
#define SEG_TYPE_FLAG_ACCESSED 1

#define SEG_TYPE_TSS (SEG_TYPE_FLAG_CODE | SEG_TYPE_FLAG_ACCESSED)

//
#if 0
#define GDT_CODE (8 * 1)
#define GDT_DATA (8 * 2)
#define GDT_USER_DATA ((8 * 3) | 3) // DPL 3
#define GDT_USER_CODE ((8 * 4) | 3) // DPL 3
#define GDT_TSS_INDEX 5
#define GDT_TSS (8 * GDT_TSS_INDEX)
#endif

/********* kernel selectors *********/
#define CODE_SELECTOR           0x08
#define CODE_64_SELECTOR        0x10
#define DATA_SELECTOR           0x18

/********* user selectors *********/
#define USER_CODE_SELECTOR      (0x20 | 3)
#define USER_DATA_SELECTOR      (0x28 | 3)
#define USER_CODE_64_SELECTOR   (0x30 | 3)

#define TSS_SELECTOR(i)            ((uint16_t)(0x38 + 16 * (i)))
