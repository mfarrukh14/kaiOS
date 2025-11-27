/*
 * KaiOS - Memory Management Header
 */

#ifndef KAIOS_MEMORY_H
#define KAIOS_MEMORY_H

#include "include/kernel/types.h"

// Memory constants
#define PAGE_SIZE 4096
#define HEAP_START 0x100000   // 1 MB
#define HEAP_SIZE  0x400000   // 4 MB heap

// Memory block header
typedef struct memory_block {
    size_t size;
    bool free;
    struct memory_block* next;
    struct memory_block* prev;
} memory_block_t;

// Memory management functions
void memory_init(void);
void* kmalloc(size_t size);
void* kcalloc(size_t num, size_t size);
void* krealloc(void* ptr, size_t size);
void kfree(void* ptr);

// Memory info
size_t memory_used(void);
size_t memory_free(void);

#endif // KAIOS_MEMORY_H
