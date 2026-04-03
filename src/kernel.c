#include <stdint.h>
#include <uart/uart.h>
#include <init/shell/shell.h>
#include <mm/pages/page.h>

void kmain(void) {

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


    uart_puts("\r\n");

    while (1) {
         shell_update();
    }
}