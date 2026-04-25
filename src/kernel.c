#include <stdint.h>
#include <uart/uart.h>
#include <init/shell/shell.h>
#include <mm/pages/page.h>
#include <mm/malloc/malloc.h>
#include <trap/trap.h>
#include <mm/mmu/mmu.h>

extern uint64_t _text_start;
extern uint64_t _text_end;
extern uint64_t _data_start;
extern uint64_t _data_end;
extern uint64_t _rodata_start;
extern uint64_t _rodata_end;
extern uint64_t _bss_start;
extern uint64_t _bss_end;
extern uint64_t _heap_start;
extern uint64_t _memory_end;
extern uint64_t _stack_start;
extern uint64_t _stack_end;

// Test function for trap
void trigger_page_fault() {
        volatile int *ptr = (int *)0x0;  // invalid address
        *ptr = 10;  // will crash
}
void kmain(void); 

void test_mmu_translation() {
    uart_puts("\r\n--- Starting MMU Translation Test ---\r\n");

    // 1. Get the current Root Table from SATP
    // We need to know which "Map" the hardware is currently using
    uint64_t satp_val = csrr(satp);
    PageTable *root = (PageTable *)((satp_val & 0xFFFFFFFFFFFFF) << 12);

    // 2. Grab a physical page to play with
    void *phys_page = page_alloc(1);
    uint64_t va = 0xDEADB000; // A "Magic" virtual address
    uint64_t pa = (uint64_t)phys_page;

    uart_printf("Mapping Virtual 0x%x to Physical 0x%x\r\n", va, pa);

    // 3. Map it!
    map(root, va, pa, PTE_R | PTE_W, 0);
    __asm__ volatile("sfence.vma zero, zero"); // Tell CPU to refresh the map

    // 4. THE PROOF: Write to the Virtual Address
    uart_puts("[1] Writing 'OS-MAGIC' to Virtual Address 0xDEADB000...\r\n");
    uint64_t *v_ptr = (uint64_t *)va;
    *v_ptr = 0x051AD1CA1; // "OS-MAGIC"

    // 5. Verify by reading the Physical Address
    uint64_t *p_ptr = (uint64_t *)pa;
    uart_printf("[2] Reading from Physical Address 0x%x: 0x%x\r\n", pa, *p_ptr);

    if (*p_ptr == 0x051AD1CA1) {
        uart_puts("[SUCCESS] MMU is successfully translating the Matrix!\r\n");
    } else {
        uart_puts("[FAILURE] MMU failed to translate.\r\n");
    }
    
    uart_puts("--- MMU Test Finished ---\r\n");
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

void kinit(void) {
   
    uart_init();
    page_init();

    // 2. Allocate and clear the Root Page Table
    PageTable *root = (PageTable *)page_alloc(1);
    for(int i = 0; i < 512; i++) root->entries[i] = 0;

    // 3. Map all kernel sections with the correct permissions
    uart_puts("Mapping Kernel...\r\n");
    // .text: Read / Execute
    id_map_range(root, (uint64_t)&_text_start, (uint64_t)&_memory_end + 0x100000, PTE_R | PTE_W | PTE_X | PTE_A | PTE_D);
    // .rodata: Read Only
    id_map_range(root, (uint64_t)&_rodata_start, (uint64_t)&_rodata_end, PTE_R);
    // .data: Read / Write
    id_map_range(root, (uint64_t)&_data_start, (uint64_t)&_data_end, PTE_R | PTE_W);
    // .bss: Read / Write
    id_map_range(root, (uint64_t)&_bss_start, (uint64_t)&_bss_end, PTE_R | PTE_W);
    // Stack: Read / Write
    id_map_range(root, (uint64_t)&_stack_start, (uint64_t)&_stack_end, PTE_R | PTE_W);
    // Heap: Read / Write
    id_map_range(root, (uint64_t)&_heap_start, (uint64_t)&_memory_end, PTE_R | PTE_W);

    
    map(root, 0x10000000, 0x10000000, PTE_R | PTE_W, 0);
    uart_puts("\r\n--- TESTING PAGE TABLE MAPPING ---\r\n");
    debug_pte(root, 0x80000566); 
    uart_puts("-----------------------------------\r\n");

    uart_puts("[+] Mapping complete. Preparing transition to S-Mode...\r\n");

    __asm__ volatile(
        "csrw pmpaddr0, %0\n\t"
        "csrw pmpcfg0, %1\n\t"
        : : "r"(0xFFFFFFFFFFFFFFFFULL), "r"(0x1F)
    );

    
    uint64_t satp_val = (8ULL << 60) | ((uint64_t)root >> 12);
    
    
    // Read mstatus and prepare it for S-Mode
    uint64_t mstatus_val;
    __asm__ volatile("csrr %0, mstatus" : "=r"(mstatus_val));
    mstatus_val &= ~(3ULL << 11); // Clear MPP
    mstatus_val |= (1ULL << 11);  // Set MPP to Supervisor
    mstatus_val |= (1ULL << 5);   // Enable Supervisor interrupts
    mstatus_val |= (3ULL << 13);  // Turn on FPU

    
    __asm__ volatile(
        "csrw satp, %0\n\t"           // Load the new Page Table
        "sfence.vma zero, zero\n\t"   // Flush the MMU cache (TLB)
        "csrw mstatus, %1\n\t"        // Set mstatus for S-Mode jump
        "csrw mepc, %2\n\t"           // Set target address to kmain
        "mret\n\t"                    // Jump!
        : 
        : "r"(satp_val), "r"(mstatus_val), "r"(kmain)
        : "memory"
    );
}

void kmain(void) {
    // void trigger_illegal_instruction() {
    //     asm volatile(".word 0xFFFFFFFF");  // invalid opcode
    // }
    // void trigger_ecall() {
    //     asm volatile("ecall");
    // }

    // uart_puts("[+] Starting OS\r\n[+] Init UART\r\n");
    // uart_init();

    // uart_puts("[+] Init Shell\r\n");
    // shell_init();

    // page_init();
    // uart_puts("[+] Init Page Allocator\r\n");
    // void* page1 = page_alloc(1);
    // uart_printf("Allocated page at: 0x%x\r\n", (uint64_t)page1);
    // void *p2 = page_alloc(1);
    // uart_puts("\r\n");
    // uart_printf("Allocated page at: 0x%x\r\n", (uint64_t)p2);
    // page_free(page1);
    // uart_puts("\r\n");
    // uart_printf("Freed page at: 0x%x\r\n", (uint64_t)page1);
    // void *page1 = page_alloc(4);
    // uart_printf("Allocated 4 contiguous pages starting at: 0x%x\r\n", (uint64_t)page1);
    // void *page2 = page_alloc(4);
    // uart_printf("Allocated 4 contiguous pages starting at: 0x%x\r\n", (uint64_t)page2);
    // void *page3 = page_alloc(4);
    // uart_printf("Allocated 4 contiguous pages starting at: 0x%x\r\n", (uint64_t)page3);
    // page_free(page3);
    // page_free(page2);
    // page_free(page1);

    // test_malloc_complex();


    // uart_puts("\r\n");

    // while (1) {
    //      shell_update();
    //     //  trigger_illegal_instruction();
    //     // trigger_ecall();
    //     // trigger_page_fault();

    // }
    // uart_puts("[SUCCESS] We are now inside kmain (Supervisor Mode)\r\n");
    // uart_puts("[INFO] MMU is active and translation is occurring.\r\n");

    // // Run your tests
    // void *page1 = page_alloc(4);
    // uart_printf("Allocated 4 pages at: 0x%x\r\n", (uint64_t)page1);
    // page_free(page1);

    // test_malloc_complex();
    
    // // NEW: Run the translation test to see the MMU "Magic"
    // test_mmu_translation(); 
    uart_puts("[SUCCESS] Hello from the Virtual Matrix (S-Mode)!\r\n");
    uart_puts("\r\n[SUCCESS] Hello from the Virtual Matrix (S-Mode)!\r\n");

    // 2. RUN THE TESTS (Uncommented!)
    uart_puts("[INFO] Running System Diagnostics...\r\n");
    
    // Test the page allocator
    void *page1 = page_alloc(4);
    uart_printf("Allocated 4 pages at: 0x%x\r\n", (uint64_t)page1);
    page_free(page1);

    // Test the Malloc engine
    test_malloc_complex();
    
    // Test the MMU Matrix Translation
    test_mmu_translation(); 

    // 3. Start the Interactive Shell
    uart_puts("[+] Starting the Interactive Shell...\r\n");
    shell_init();

    while (1) {
         shell_update();
    }
}