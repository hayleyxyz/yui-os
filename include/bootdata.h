#pragma once

#include <types.h>

#define BOOTDATA_MAGIC 0x01caf4b5

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

struct bootdata {
    uint32_t magic;
    struct mmap * mmap;
    uint32_t mmap_count;
} __attribute__((packed));