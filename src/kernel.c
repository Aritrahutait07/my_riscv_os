#include <stdint.h>
#include <uart/uart.h>
#include <init/shell/shell.h>
#include <mm/pages/page.h>
#include <mm/malloc/malloc.h>
#include <trap/trap.h>


// Test function for trap
void trigger_page_fault() {
        volatile int *ptr = (int *)0x0;  // invalid address
        *ptr = 10;  // will crash
}

// Test for malloc and free
void test_malloc_complex() {
    uart_puts("\r\n--- Starting Complex Malloc Test ---\r\n");

    // 1. Test: Multiple Small Allocations
    uart_puts("[1] Allocating 3 small blocks (A, B, C)...\r\n");
    char *a = (char *)malloc(100);
    char *b = (char *)malloc(100);
    char *c = (char *)malloc(100);

    if (a && b && c) {
        uart_printf("  A: 0x%x, B: 0x%x, C: 0x%x\r\n", (uint64_t)a, (uint64_t)b, (uint64_t)c);
    }

    // 2. Test: Slicing & Hole Reuse
    uart_puts("[2] Freeing B and allocating D (size 50) to see if it slices B's hole...\r\n");
    free(b); 
    char *d = (char *)malloc(50);
    uart_printf("  D (50 bytes) allocated at: 0x%x (Should match B's old address)\r\n", (uint64_t)d);

    // 3. Test: The "Healing" (Coalescing) Test
    // If we free A and then free D (which is in B's old spot), 
    // and then free C... they should all merge back into one giant block.
    uart_puts("[3] Freeing A, D, and C to trigger merging...\r\n");
    free(a);
    free(d);
    free(c);

    uart_puts("[4] Allocating a large block (300 bytes) that requires the merged space...\r\n");
    char *large = (char *)malloc(300);
    if (large) {
        uart_printf("  Large block allocated at: 0x%x (Success! Merging works)\r\n", (uint64_t)large);
        free(large);
    } else {
        uart_puts("  FAILED: Merging did not create a 300-byte block.\r\n");
    }

    // 4. Test: Cross-Page Allocation
    uart_puts("[5] Requesting 5000 bytes (Crosses 4KB page boundary)...\r\n");
    void *big_ptr = malloc(5000);
    if (big_ptr) {
        uart_printf("  Big block (5000 bytes) at: 0x%x\r\n", (uint64_t)big_ptr);
        free(big_ptr);
    } else {
        uart_puts("  FAILED: Could not allocate across page boundary.\r\n");
    }

    // 5. Test: Stress Test - Exhausting the Leftover Array
    uart_puts("[6] Stress test: 10 small allocations...\r\n");
    void *ptrs[10];
    for(int i=0; i<10; i++) {
        ptrs[i] = malloc(16);
    }
    uart_puts("  Freeing stress test blocks...\r\n");
    for(int i=0; i<10; i++) {
        free(ptrs[i]);
    }

    uart_puts("--- Complex Malloc Test Finished ---\r\n\r\n");
}

void kmain(void) {
    // void trigger_illegal_instruction() {
    //     asm volatile(".word 0xFFFFFFFF");  // invalid opcode
    // }
    // void trigger_ecall() {
    //     asm volatile("ecall");
    // }

    uart_puts("[+] Starting OS\r\n[+] Init UART\r\n");
    uart_init();

    uart_puts("[+] Init Shell\r\n");
    shell_init();

    page_init();
    uart_puts("[+] Init Page Allocator\r\n");
    // void* page1 = page_alloc(1);
    // uart_printf("Allocated page at: 0x%x\r\n", (uint64_t)page1);
    // void *p2 = page_alloc(1);
    // uart_puts("\r\n");
    // uart_printf("Allocated page at: 0x%x\r\n", (uint64_t)p2);
    // page_free(page1);
    // uart_puts("\r\n");
    // uart_printf("Freed page at: 0x%x\r\n", (uint64_t)page1);
    void *page1 = page_alloc(4);
    uart_printf("Allocated 4 contiguous pages starting at: 0x%x\r\n", (uint64_t)page1);
    void *page2 = page_alloc(4);
    uart_printf("Allocated 4 contiguous pages starting at: 0x%x\r\n", (uint64_t)page2);
    void *page3 = page_alloc(4);
    uart_printf("Allocated 4 contiguous pages starting at: 0x%x\r\n", (uint64_t)page3);
    page_free(page3);
    page_free(page2);
    page_free(page1);

    test_malloc_complex();


    uart_puts("\r\n");

    while (1) {
         shell_update();
        //  trigger_illegal_instruction();
        // trigger_ecall();
        trigger_page_fault();

    }
}