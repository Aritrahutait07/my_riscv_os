#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>

// Page size in beytes = 4KB
#define PAGE_SIZE 4096

#define PAGE_TAKEN (1 << 0) // Example flag to indicate if the page is allocated
#define PAGE_LAST  (1 << 1) // Example flag to indicate if this is the last page in a contiguous allocation (useful for future enhancements)

// Page structure pointing to the next page in the free list
// struct Page{
//     struct Page* next;
// };
// Moving to the approach of descriptor array to avoid the need for dynamic memory allocation for the free list nodes
typedef struct {
    uint8_t flags; // can be used for future enhancements (e.g., to track allocated vs free pages)
}PageDescriptor;




// Page initialization function
void page_init(void);
// Page allocation function, returns a pointer to the allocated page
void* page_alloc(int n);
// Page deallocation function, takes a pointer to the page to be freed
void page_free(void* ptr);

#endif


