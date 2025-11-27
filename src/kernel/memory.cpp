/*
 * KaiOS - Memory Management Implementation
 * Simple heap allocator with linked list
 */

#include "include/kernel/memory.h"
#include "include/kernel/string.h"

// Heap memory area
static uint8_t heap_memory[HEAP_SIZE];
static memory_block_t* heap_start = NULL;
static size_t total_allocated = 0;

void memory_init(void) {
    // Initialize the first block covering the entire heap
    heap_start = (memory_block_t*)heap_memory;
    heap_start->size = HEAP_SIZE - sizeof(memory_block_t);
    heap_start->free = true;
    heap_start->next = NULL;
    heap_start->prev = NULL;
    total_allocated = 0;
}

// Find a free block of at least the requested size
static memory_block_t* find_free_block(size_t size) {
    memory_block_t* current = heap_start;
    
    while (current != NULL) {
        if (current->free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

// Split a block if it's larger than needed
static void split_block(memory_block_t* block, size_t size) {
    // Only split if remaining space is large enough for a new block
    if (block->size >= size + sizeof(memory_block_t) + 16) {
        memory_block_t* new_block = (memory_block_t*)((uint8_t*)(block + 1) + size);
        new_block->size = block->size - size - sizeof(memory_block_t);
        new_block->free = true;
        new_block->next = block->next;
        new_block->prev = block;
        
        if (block->next != NULL) {
            block->next->prev = new_block;
        }
        
        block->next = new_block;
        block->size = size;
    }
}

// Merge adjacent free blocks
static void merge_blocks(memory_block_t* block) {
    // Merge with next block if free
    if (block->next != NULL && block->next->free) {
        block->size += sizeof(memory_block_t) + block->next->size;
        block->next = block->next->next;
        if (block->next != NULL) {
            block->next->prev = block;
        }
    }
    
    // Merge with previous block if free
    if (block->prev != NULL && block->prev->free) {
        block->prev->size += sizeof(memory_block_t) + block->size;
        block->prev->next = block->next;
        if (block->next != NULL) {
            block->next->prev = block->prev;
        }
    }
}

void* kmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    // Align size to 8 bytes
    size = (size + 7) & ~7;
    
    memory_block_t* block = find_free_block(size);
    
    if (block == NULL) {
        return NULL;  // Out of memory
    }
    
    split_block(block, size);
    block->free = false;
    total_allocated += block->size;
    
    // Return pointer to data area (after the header)
    return (void*)(block + 1);
}

void* kcalloc(size_t num, size_t size) {
    size_t total = num * size;
    void* ptr = kmalloc(total);
    
    if (ptr != NULL) {
        memset(ptr, 0, total);
    }
    
    return ptr;
}

void* krealloc(void* ptr, size_t size) {
    if (ptr == NULL) {
        return kmalloc(size);
    }
    
    if (size == 0) {
        kfree(ptr);
        return NULL;
    }
    
    memory_block_t* block = (memory_block_t*)ptr - 1;
    
    // If the current block is big enough, just return
    if (block->size >= size) {
        return ptr;
    }
    
    // Allocate new block and copy data
    void* new_ptr = kmalloc(size);
    if (new_ptr != NULL) {
        memcpy(new_ptr, ptr, block->size);
        kfree(ptr);
    }
    
    return new_ptr;
}

void kfree(void* ptr) {
    if (ptr == NULL) {
        return;
    }
    
    memory_block_t* block = (memory_block_t*)ptr - 1;
    
    if (block->free) {
        return;  // Already freed (double-free protection)
    }
    
    total_allocated -= block->size;
    block->free = true;
    
    // Merge with adjacent free blocks
    merge_blocks(block);
}

size_t memory_used(void) {
    return total_allocated;
}

size_t memory_free(void) {
    size_t free_mem = 0;
    memory_block_t* current = heap_start;
    
    while (current != NULL) {
        if (current->free) {
            free_mem += current->size;
        }
        current = current->next;
    }
    
    return free_mem;
}
