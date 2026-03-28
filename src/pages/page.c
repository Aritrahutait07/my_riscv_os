#include "page.h"
#include <uart/uart.h>
#ifndef NULL
#define NULL ((void*)0)
#endif

extern uint64_t _bss_end; // where kernel global zero-initialized memory ends
extern uint64_t _stack_end; // where kernel stack ends

static struct Page *page_free_list_head = NULL; // head of the free page list

void page_init(void){
    uint64_t start = (uint64_t)(&_bss_end); // start of free memory
    uint64_t end = 0x80000000 + (128 * 1024 * 1024); // end of physical memory (128MB)
    start = (start + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1); // align start to the next page boundary
    for(uint64_t i = start ; i< end; i += PAGE_SIZE){
        page_free((void*)i); // add each page to the free list
    }
}
//  n unused for now, but can be used to allocate multiple contiguous pages in the future
// for now just ignore warning about it
void* page_alloc(int n){
    if(page_free_list_head == NULL){
        uart_puts("Out of memory!\n");
        return NULL; // no free pages available
    }
    struct Page* page = page_free_list_head; // get the head of the free list
    page_free_list_head = page->next; // move the head to the next free page
    return (void*)page; // return the allocated page
}

void page_free(void* page){
    struct Page* new_page = (struct Page*)page; // create a new page structure
    new_page->next = page_free_list_head; // point to the current head of the free list
    page_free_list_head = new_page; // update the head to the newly freed page
}
