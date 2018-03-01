#pragma once

#include <types.h>

#define BOOTDATA_MAGIC 0xb007da7a

struct mmap {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
#define MMAP_MEMORY_AVAILABLE              1
#define MMAP_MEMORY_RESERVED               2
#define MMAP_MEMORY_ACPI_RECLAIMABLE       3
#define MMAP_MEMORY_NVS                    4
#define MMAP_MEMORY_BADRAM                 5
  uint32_t type;
} __attribute__((packed));

// Used to transfer data from the protected mode 32-bit initial boot loader, to the 64-bit kernel.
// uintptr64_t should be used for all pointers
struct bootdata {
    uint32_t magic;
    uint32_t mmap_count;
    uintptr64_t mmap;
    uintptr64_t pml4;
    uintptr64_t pdp;
    uintptr64_t pte;
    uintptr64_t gdt;
    uintptr64_t gdt_tss;
} __attribute__((packed));

#define EXTEND_POINTER(ptr) ((uintptr64_t)((uintptr_t)ptr))