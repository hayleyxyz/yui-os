#include <memory.h>
#include <console.h>
#include <multiboot.h>

/* top level kernel page tables, initialized in start.S */
volatile uint64_t pml4[NO_OF_PT_ENTRIES] __attribute__((aligned(4096)));
volatile uint64_t pdp[NO_OF_PT_ENTRIES] __attribute__((aligned(4096))); /* temporary */
volatile uint64_t pte[NO_OF_PT_ENTRIES * NO_OF_PT_ENTRIES] __attribute__((aligned(4096)));


// TODO: redo this
uint8_t _memory_block_backing[sizeof(struct memory_block) * 128];
struct memory_block * head_memory_block;


static void * map_page(uint64_t physical_addr, uint64_t virtual_addr) {
    uint64_t pd_index = (virtual_addr / PAGE_SIZE_2MB) / NO_OF_PT_ENTRIES;
    uint64_t pt_index = (pd_index * NO_OF_PT_ENTRIES) + ((virtual_addr / PAGE_SIZE_2MB) % NO_OF_PT_ENTRIES);

    if((pdp[pd_index] & PG_PRESENT) == 0) {
        pdp[pd_index] = (uint64_t)&pte[pt_index] | PG_PRESENT | PG_RW | PG_USER;
    }

    pte[pt_index] = physical_addr | PG_PRESENT | PG_RW | PG_PSE | PG_USER;

    return (void *)virtual_addr;
}


static struct memory_block * memory_block_new(uintptr_t address, uint64_t length, bool available) {
    struct memory_block * block = head_memory_block;

    while(block->magic == MEMORY_BLOCK_MAGIC) {
        block++;
    }

    block->magic = MEMORY_BLOCK_MAGIC;
    block->address = address;
    block->length = length;
    block->available = available;

    return block;
}

static bool memory_split_block(uintptr_t address, uint64_t length) {
    struct memory_block * block = head_memory_block,
        * alloc_block,
        * remainder;

    while(block) {
        if(block->address <= address && (block->address + block->length) > (address + length)) {
            remainder = memory_block_new(
                            (address + length),
                            (block->address + block->length) - (address + length),
                            true);

            alloc_block = memory_block_new(address, length, false);

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
                    remainder->next = block->next;
                    remainder->next->prev = remainder;
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

                if(head_memory_block == block) {
                    head_memory_block = alloc_block;
                }
            }

            return true;
        }

        block = block->next;
    }

    return false;
}

void * alloc_page() {
    struct memory_block * block = head_memory_block;
    uintptr_t aligned = 0;
    uint64_t length = memory_page_size();

    while(block) {
        aligned = ALIGN_UP(block->address, length);

        if(block->available && (aligned - block->address) - block->length >= length) {
            memory_split_block(aligned, length);
            return map_page(aligned, aligned);
        }

        block = block->next;
    }

    return nullptr;
}

extern void __code_start;
extern void _end;

void init_memory(struct multiboot_info * mb_info) {
    struct multiboot_mmap_entry * mmap_entry = (struct multiboot_mmap_entry *)mb_info->mmap_addr,
        * largest_block = nullptr;
    uint32_t i;

    for(i = 0; i < mb_info->mmap_length / sizeof(struct multiboot_mmap_entry); i++) {
        if(mmap_entry[i].type == MULTIBOOT_MEMORY_AVAILABLE &&
            (!largest_block || largest_block->len < mmap_entry[i].len)) {
            largest_block = &mmap_entry[i];
        }
    }

    uintptr_t kernel_start = (uintptr_t)&__code_start;
    uintptr_t kernel_end = (uintptr_t)&_end;
    uint64_t kernel_length = kernel_end - kernel_start;

    head_memory_block = (struct memory_block *)&_memory_block_backing;

    struct memory_block * block_a = nullptr,
        * block_b = nullptr;

    if(largest_block->addr == kernel_start) { // very likely to be the case
        block_a = memory_block_new(kernel_start, kernel_length, false);
        block_b = memory_block_new(kernel_end, largest_block->len - kernel_length, true);

        block_a->next = block_b;
        block_b->prev = block_a;
    }
    else {
        memory_block_new(largest_block->addr, largest_block->len, true);
        memory_split_block(kernel_start, kernel_end);
    }
}

void dump_memory() {
    struct memory_block * block = head_memory_block;

    printk("DUMPING MEMORY\n");

    while(block) {
        printk("addr: 0x%016llx len: 0x%016llx, avil: %c\n",
            block->address, block->length, block->available ? 'Y' : 'N');

        block = block->next;
    }
}