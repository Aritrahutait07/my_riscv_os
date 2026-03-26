#include <stdint.h>
#include <uart/uart.h>
#include <shell/shell.h>

void kmain(void) {

    uart_puts("[+] Starting OS\r\n[+] Init UART\r\n");
    uart_init();

    uart_puts("[+] Init Shell\r\n");
    shell_init();

    while (1) {
         shell_update();
    }
}