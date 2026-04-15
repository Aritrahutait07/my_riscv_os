#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>

// Page size in bytes = 4KB
#define PAGE_SIZE 4096

#define PAGE_TAKEN (1 << 0) // Example flag to indicate if the page is allocated

// Page structure pointing to the next page in the free list
// struct Page{
//     struct Page* next;
// };
// Moving to the approach of descriptor array to avoid the need for dynamic memory allocation for the free list nodes
typedef struct {
    uint8_t flags;       // 1 if taken, 0 if free
    uint32_t block_size; // Stores 'n'. Only the FIRST page of a block needs this!
} PageDescriptor;




// Page initialization function
void page_init(void);
// Page allocation function, returns a pointer to the allocated page
void* page_alloc(int n);
// Page deallocation function, takes a pointer to the page to be freed
void page_free(void* ptr);

uint64_t page_get_heap_start(void);
PageDescriptor* page_get_descriptors(void);
int get_page_index_from_address(void* addr);

#endif


