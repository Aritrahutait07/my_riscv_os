#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>

// Page size in beytes = 4KB
#define PAGE_SIZE 4096

// Page structure pointing to the next page in the free list
struct Page{
    struct Page* next;
};

// Page initialization function
void page_init(void);
// Page allocation function, returns a pointer to the allocated page
void* page_alloc(int n);
// Page deallocation function, takes a pointer to the page to be freed
void page_free(void* page);

#endif


