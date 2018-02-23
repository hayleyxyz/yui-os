#include <types.h>
#include <bootdata.h>
#include <console.h>
#include <cpu.h>

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
};

struct memory_block * head_memory_block;

static void memory_split(uintptr_t addr, uint64_t length) {
    struct memory_block * block = (struct memory_block *)head_memory_block;

    block = (struct memory_block *)head_memory_block;

    while(block) {
        if(block->address <= addr && (block->address + block->length) >= addr + length && block->available) {
            break;
        }

        block = block->next;
    }

    console_printf("addr: 0x%08x%08x len: 0x%08x%08x available: %c\n",
            block->address >> 32, block->address & 0xffffffff, block->length >> 32,
            block->length & 0xffffffff, block->available ? 'y' : 'n');
}

static void init_memory(struct bootdata * bootdata) {
    pml4 = bootdata->pml4;
    pdp = bootdata->pdp;
    pte = bootdata->pte;

    struct mmap * mmap = (struct mmap *)bootdata->mmap,
        * suitable_block = nullptr;
    for(uint32_t i = 0; i < bootdata->mmap_count; i++) {
        if(mmap[i].type == MMAP_MEMORY_AVAILABLE && mmap[i].len >= (PAGE_SIZE_2MB * 5)) {
            if(mmap[i].addr != 0) { // Temp check until I can relocate kernel address
                suitable_block = &mmap[i];
            }
            break;
        }
    }

    uintptr_t addr = ALIGN_UP(suitable_block->addr, PAGE_SIZE_2MB);

    void * allocated = map_page(addr, addr);
    head_memory_block = nullptr;

    struct memory_block * block = (struct memory_block *)allocated,
        * prev = nullptr;
    for(uint32_t i = 0; i < bootdata->mmap_count; i++) {
        if(mmap[i].type == MMAP_MEMORY_AVAILABLE) {
            block->address = mmap[i].addr;
            block->length = mmap[i].len;
            block->available = true;
            block->next = nullptr;
            
            if(prev != nullptr) {
                prev->next = block;
            }
            else if(head_memory_block == nullptr) {
                head_memory_block = block;
            }

            prev = block;
            block++;
        }
    }

    //

    memory_split(allocated, PAGE_SIZE_2MB);

    block = (struct memory_block *)head_memory_block;

    while(block) {
        #if 0
        console_printf("addr: 0x%08x%08x len: 0x%08x%08x available: %c\n",
            block->address >> 32, block->address & 0xffffffff, block->length >> 32,
            block->length & 0xffffffff, block->available ? 'y' : 'n');
        #endif

        block = block->next;
    }
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
        console_printf("addr: 0x%08x%08x len: 0x%08x%08x type: 0x%08x\n",
            mmap[i].addr >> 32,
            mmap[i].addr & 0xffffffff,
            mmap[i].len >> 32,
            mmap[i].len & 0xffffffff,
            mmap[i].type);
    }

    init_memory(bootdata);
}