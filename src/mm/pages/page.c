#include "page.h"
#include <uart/uart.h>
#ifndef NULL
#define NULL ((void*)0)
#endif


extern uint64_t _bss_end; // where kernel global zero-initialized memory ends
extern uint64_t _stack_end; // where kernel stack ends
extern uint64_t _heap_start; // where kernel heap starts
extern uint64_t _heap_size; // size of the kernel heap
static PageDescriptor *ptr_to_descriptaors; // pointer to the start of the page descriptors array
static uint32_t total_pages; // total number of pages available in the system
static uint64_t actual_heap_start; // actual start of the heap after the page descriptors


//static struct Page *page_free_list_head = NULL; // head of the free page list

void page_init(void){
    // uint64_t start = (uint64_t)(&_bss_end); // start of free memory
    // uint64_t end = 0x80000000 + (128 * 1024 * 1024); // end of physical memory (128MB)
    // start = (start + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1); // align start to the next page boundary
    // for(uint64_t i = start ; i< end; i += PAGE_SIZE){
    //     page_free((void*)i); // add each page to the free list
    // }
    uint64_t h_start = (uint64_t)&_heap_start;
    uint64_t h_size  = (uint64_t)&_heap_size;
    h_start = (h_start + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    total_pages =(uint32_t) h_size / PAGE_SIZE; // calculate total number of pages available
    ptr_to_descriptaors = (PageDescriptor*)h_start; // point to the start of the page descriptors array
    uint64_t descriptors_size = total_pages * sizeof(PageDescriptor); // calculate the size of the page descriptors array
    uint64_t descriptors_pages = (descriptors_size + PAGE_SIZE - 1) / PAGE_SIZE; // calculate how many pages are needed for the descriptors
    actual_heap_start = (h_start + (descriptors_pages * PAGE_SIZE) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);// calculate the actual start of the heap after the page descriptors
    uint64_t overhead = actual_heap_start - h_start; // calculate the overhead caused by the page descriptors
    total_pages -= (uint32_t)(overhead / PAGE_SIZE); // reduce the total number of pages available for allocation by the number of pages used for the descriptors
    for(uint32_t i = 0; i < total_pages; i++){
        ptr_to_descriptaors[i].flags = 0; // initialize all page descriptors as free
        //page_free((void*)(actual_heap_start + (i * PAGE_SIZE))); // add each page to the free list
        ptr_to_descriptaors[i].block_size = 0; // initialize block size to 0 for all pages
    }
    uart_puts("[+] Page Allocator Initialized\r\n");
    




}
//  n unused for now, but can be used to allocate multiple contiguous pages in the future
// for now just ignore warning about it
void* page_alloc(int n){
    if (n <= 0) return NULL; // invalid request
    uint32_t i = 0;

    while(i<=total_pages-n){
    // if we hit a start block of taken pages, skip the entire block
       if (ptr_to_descriptaors[i].flags & PAGE_TAKEN)
       {
            i += ptr_to_descriptaors[i].block_size;
            continue;
       }
       
       int found = 1;
       uint32_t j;

       for(j= i; j < i+n; j++){
            if (ptr_to_descriptaors[j].flags & PAGE_TAKEN)
            {
                found = 0;
                break;
            }
       }

        if(found){
            for(j = i;j<i+n;j++){
                ptr_to_descriptaors[j].flags |= PAGE_TAKEN; // mark the pages as taken
            }
            ptr_to_descriptaors[i].block_size = n; // store the block size in the first page descriptor of the allocated block
            return (void*)(actual_heap_start + (i * PAGE_SIZE)); // return the starting address of the allocated pages
        }
        else {
            i = j; // skip the checked pages and continue searching
        }
    }
    uart_puts("Out of memory!\n");
    return NULL; // no contiguous block of n free pages found
}

void page_free(void* p){
    if (p == NULL) return;
    uint64_t addr = (uint64_t)p;
    uint32_t i = (uint32_t)((addr - actual_heap_start) / PAGE_SIZE);

    if (!(ptr_to_descriptaors[i].flags & PAGE_TAKEN)) {
        uart_printf("ERROR: Double free or invalid pointer at index %d!\r\n", i);
        return;
    }

    uint32_t block_size = ptr_to_descriptaors[i].block_size; // get the block size from the first page descriptor of the block

    uart_printf("Freeing page at index: %d\r\n", i);
    for(uint32_t k = 0; k < block_size; k++){
        ptr_to_descriptaors[i+k].flags &= ~PAGE_TAKEN; // mark all pages in the block as free
        ptr_to_descriptaors[i+k].block_size = 0; // reset the block size for all pages in the block
    }
}