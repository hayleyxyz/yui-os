#pragma once

#include <types.h>
#include <bootdata.h>

#define NO_OF_PT_ENTRIES        512
#define PAGE_SIZE_2MB           0x200000
#define PG_PRESENT  (1 << 0)
#define PG_RW       (1 << 1)
#define PG_PSE      (1 << 7)

#define ALIGN_UP(addr, align) \
    ((addr + (align - 1)) & -align)

struct memory_block {
    uintptr_t address;
    uint64_t length;
    bool available;
    struct memory_block * next;
    struct memory_block * prev;
#define MEMORY_BLOCK_MAGIC 0xabd0dbca
    uint32_t magic;
};


void init_memory(struct bootdata * bootdata);
void * alloc_page();

static inline uint32_t memory_page_size() {
    return PAGE_SIZE_2MB;
}