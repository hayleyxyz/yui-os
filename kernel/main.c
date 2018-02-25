#include <types.h>
#include <bootdata.h>
#include <console.h>
#include <cpu.h>
#include <debug.h>

volatile uint64_t * pml4;
volatile uint64_t * pdp;
volatile uint64_t * pte;

#define NO_OF_PT_ENTRIES        512
#define PAGE_SIZE_2MB           0x200000
#define PG_PRESENT  (1 << 0)
#define PG_RW       (1 << 1)
#define PG_PSE      (1 << 7)

#define ALIGN_UP(addr, align) \
    ((addr + (align - 1)) & -align)

static void * map_page(uint64_t physical_addr, uint64_t virtual_addr) {
    uint64_t pd_index = (virtual_addr / PAGE_SIZE_2MB) / NO_OF_PT_ENTRIES;
    uint64_t pt_index = (pd_index * NO_OF_PT_ENTRIES) + ((virtual_addr / PAGE_SIZE_2MB) % NO_OF_PT_ENTRIES);

    if((pdp[pd_index] & PG_PRESENT) == 0) {
        pdp[pd_index] = (uint64_t)&pte[pt_index] | PG_PRESENT | PG_RW;
    }

    pte[pt_index] = physical_addr | PG_PRESENT | PG_RW | PG_PSE;

    return (void *)virtual_addr;
}

struct memory_block {
    uintptr_t address;
    uint64_t length;
    bool available;
    struct memory_block * next;
    struct memory_block * prev;
    uint32_t magic;
};

#define MEMORY_BLOCK_MAGIC 0xabd0dbca

struct memory_block * head_memory_block;

static inline void dump_blocks() {
    struct memory_block * block = head_memory_block;

    console_puts("\nDUMP MEM. BLOCKS\n");

    while(block) {
        printk("addr: 0x%08x%08x len: 0x%08x%08x available: %c\n",
                block->address >> 32, block->address & 0xffffffff, block->length >> 32,
                block->length & 0xffffffff, block->available ? 'y' : 'n');

        block = block->next;
    }
}

static struct memory_block * memory_block_new() {
    struct memory_block * block = head_memory_block;

    while(block->magic == MEMORY_BLOCK_MAGIC) {
        block++;
    }

    block->magic = MEMORY_BLOCK_MAGIC;

    return block;
}

static inline void link_memory_blocks(struct memory_block * first, struct memory_block * second) {
    first->next = second;
    second->prev = first;
}

static void memory_split_block(uintptr_t address, uint64_t length) {
    struct memory_block * block = head_memory_block,
        * alloc_block,
        * remainder;

    printk("memory_split_block(0x%08x%08x, 0x%08x%08x)\n",
        address >> 32, address, length >> 32, length);

    while(block) {
        if(block->address <= address && (block->address + block->length) > (address + length)) {
            remainder = memory_block_new();
            remainder->address = (address + length);
            remainder->length = (block->address + block->length) - remainder->address;
            remainder->available = true;

            alloc_block = memory_block_new();
            alloc_block->address = address;
            alloc_block->length = length;
            alloc_block->available = false;

            if(block->address < address) {
                // block | alloc | remainder
                block->length = address - block->address;
                alloc_block->prev = block;
                alloc_block->next = remainder;
                remainder->prev = alloc_block;

                if(!block->next) {
                    block->next = alloc_block;
                }
                else {
                    not_implemented();
                }
            }
            else if(block->address == address) {
                // alloc | remainder
                block->length = (block->address + block->length) - (address + length);
                block->address = (address + length);


                alloc_block->prev = block->prev;
                alloc_block->prev->next = alloc_block;
                alloc_block->next = block;
                block->prev = alloc_block;

            }

            break;
        }

        block = block->next;
    }
}

static void init_memory(struct bootdata * bootdata) {
    pml4 = (uint64_t *)bootdata->pml4;
    pdp = (uint64_t *)bootdata->pdp;
    pte = (uint64_t *)bootdata->pte;

    struct mmap * mmap = (struct mmap *)bootdata->mmap,
        * largest_block = nullptr;
    for(uint32_t i = 0; i < bootdata->mmap_count; i++) {
        if(mmap[i].type == MMAP_MEMORY_AVAILABLE && (!largest_block || largest_block->len < mmap[i].len)) {
            largest_block = &mmap[i];
        }
    }

    uintptr_t addr = ALIGN_UP(largest_block->addr, PAGE_SIZE_2MB);
    void * allocated = map_page(addr, addr);
    head_memory_block = (struct memory_block *)allocated;

    head_memory_block->address = largest_block->addr;
    head_memory_block->length = largest_block->len;
    head_memory_block->available = true;
    head_memory_block->next = nullptr;
    head_memory_block->magic = MEMORY_BLOCK_MAGIC;

    // Mark the block we just allocated
    memory_split_block(addr, PAGE_SIZE_2MB);
    memory_split_block(0x400000, PAGE_SIZE_2MB);
    memory_split_block(0x23400000, PAGE_SIZE_2MB);

    dump_blocks();
}

void kernel_main(struct bootdata * bootdata) {
    console_clear();
    console_puts("yui-os KERNEL v0.0.1\n");

    if(bootdata->magic != BOOTDATA_MAGIC) {
        console_puts("Bootdata magic invalid!");
        halt();
    }

    struct mmap * mmap = (struct mmap *)bootdata->mmap;
    for(uint32_t i = 0; i < bootdata->mmap_count; i++) {
        #if 0
        printk("addr: 0x%08x%08x len: 0x%08x%08x type: 0x%08x\n",
            mmap[i].addr >> 32,
            mmap[i].addr & 0xffffffff,
            mmap[i].len >> 32,
            mmap[i].len & 0xffffffff,
            mmap[i].type);
        #endif
    }

    init_memory(bootdata);
}