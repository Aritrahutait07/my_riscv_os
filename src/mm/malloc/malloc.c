#include <stddef.h>
#include <mm/malloc/malloc.h>
#include <mm/pages/page.h>
#include <uart/uart.h>

#define MAX_LEFTOVER_BLOCKS 256  

static m_lfov leftover_blocks[MAX_LEFTOVER_BLOCKS];
static uint32_t num_leftover_blocks = 0;

void* allocate_leftover_pages(size_t size, m_lfov* leftover_blocks) {
    
    for (uint32_t i = 0; i < num_leftover_blocks; i++) {
        
        // First-Fit: Does this block have enough space?
        if (leftover_blocks[i].free_size >= size) {
            
            // Save the starting address of the block we are taking
            void* allocated_ptr = (void*)leftover_blocks[i].left_over_start;

            if (leftover_blocks[i].free_size == size) {
                // EXACT MATCH: The block is exactly the size we need.
                // We consume the whole block, so remove it from the array.
                for (uint32_t j = i; j < num_leftover_blocks - 1; j++) {
                    leftover_blocks[j] = leftover_blocks[j + 1];
                }
                num_leftover_blocks--; 
            } else {
                // SLICING: The block is bigger than we need.
                // Take 'size' bytes from the beginning, and shrink the leftover block.
                leftover_blocks[i].left_over_start += size;
                leftover_blocks[i].free_size -= size;
            }
            
            return allocated_ptr;
        }
    }
    
    // If the loop finishes, no leftover block was big enough.
    return NULL; 
}

void* malloc(size_t size) {
    if (size == 0) return NULL;

    // 1. ADD THE HEADER SIZE TO THE REQUEST
    size_t actual_size_needed = size + sizeof(MallocHeader);

    // 2. Try to get it from leftovers first
    void* allocated_block = allocate_leftover_pages(actual_size_needed, leftover_blocks);

    // 3. If we didn't find a leftover, we must allocate new pages
    if (allocated_block == NULL) {
        
        // This ONE formula perfectly calculates how many pages we need, 
        // whether it's 1 page, exactly 2 pages, or 3.5 pages!
        uint32_t num_pages = (actual_size_needed + PAGE_SIZE - 1) / PAGE_SIZE; 
        
        allocated_block = page_alloc(num_pages); 
        if (allocated_block == NULL) return NULL; // Out of memory!

        // Calculate the leftover space from the new pages
        uint32_t total_allocated_space = num_pages * PAGE_SIZE;
        uint32_t leftover_space = total_allocated_space - actual_size_needed; 

        if (leftover_space > 0) {
            uint64_t left_over_addr = (uint64_t)allocated_block + actual_size_needed; 
            
            if (num_leftover_blocks < MAX_LEFTOVER_BLOCKS) {
                leftover_blocks[num_leftover_blocks].left_over_start = left_over_addr;
                leftover_blocks[num_leftover_blocks].free_size = leftover_space;
                num_leftover_blocks++;
            } else {
                uart_puts("WARNING: Max leftover blocks reached.\n");
            }
        }
    }

    // --- AT THIS POINT, WE HAVE RAW MEMORY ---
    // It doesn't matter if it came from leftovers or fresh pages.
    // 'allocated_block' points to a chunk of memory big enough for our data + header.

    // 4. WRITE THE HEADER AND RETURN THE POINTER
    MallocHeader* header = (MallocHeader*)allocated_block;
    
    // Store ONLY the user's requested size (so free() knows how much data they used)
    header->size = size; 

    // Return the memory address right AFTER the header
    return (void*)(header + 1); 
}

void free(void* ptr) {
    if (ptr == NULL) return;

    // 1. READ THE HEADER
    // Go backwards in memory by the size of the header to find our metadata
    MallocHeader* header = (MallocHeader*)ptr - 1;
    
    // The total size we are freeing includes the header itself!
    uint32_t total_freed_size = header->size + sizeof(MallocHeader);
    uint64_t freed_address = (uint64_t)header;

    // 2. ADD TO LEFTOVER BLOCKS
    if (num_leftover_blocks < MAX_LEFTOVER_BLOCKS) {
        leftover_blocks[num_leftover_blocks].left_over_start = freed_address;
        leftover_blocks[num_leftover_blocks].free_size = total_freed_size;
        num_leftover_blocks++;
    } else {
        uart_puts("ERROR: Leftover blocks array is full! Memory leak occurred.\n");
        return;
    }

    // 3. SORT THE ARRAY (Bubble Sort)
    // We sort by memory address from lowest to highest.
    for (uint32_t i = 0; i < num_leftover_blocks - 1; i++) {
        for (uint32_t j = 0; j < num_leftover_blocks - i - 1; j++) {
            if (leftover_blocks[j].left_over_start > leftover_blocks[j + 1].left_over_start) {
                // Swap the blocks
                m_lfov temp = leftover_blocks[j];
                leftover_blocks[j] = leftover_blocks[j + 1];
                leftover_blocks[j + 1] = temp;
            }
        }
    }

    // 4. COALESCE (MERGE) ADJACENT BLOCKS
    // Because it is sorted, any blocks that are physically touching 
    // will be right next to each other in the array!
    for (uint32_t i = 0; i < num_leftover_blocks - 1; ) {
        uint64_t current_end = leftover_blocks[i].left_over_start + leftover_blocks[i].free_size;
        
        // If the end of this block is exactly the start of the next block...
        if (current_end == leftover_blocks[i + 1].left_over_start) {
            
            // Merge them! Add the next block's size to the current one.
            leftover_blocks[i].free_size += leftover_blocks[i + 1].free_size;
            
            // Shift all the other blocks down to remove the duplicate entry
            for (uint32_t k = i + 1; k < num_leftover_blocks - 1; k++) {
                leftover_blocks[k] = leftover_blocks[k + 1];
            }
            num_leftover_blocks--; 
            
            // Note: We DO NOT do i++ here. We want to check if the newly merged 
            // block also touches the *next* block in the array!
        } else {
            i++; // They didn't touch, safely move to the next one.
        }
    }
}