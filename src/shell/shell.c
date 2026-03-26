#include "shell.h"
#include "../uart/uart.h"

#define MAX_BUF 128
static char buffer[MAX_BUF];
static int index = 0;

// Simple string comparison
static int strncmp(const char *s1, const char *s2, int n) {
    while (n-- > 0 && *s1 && (*s1 == *s2)) { s1++; s2++; }
    return (n == -1) ? 0 : (*(unsigned char *)s1 - *(unsigned char *)s2);
}

static void execute_command(char *line) {
    uart_puts("\r\n");
    
    if (strncmp(line, "echo ", 5) == 0) {
        uart_puts(line + 5); // Print everything after "echo "
    } else if (strncmp(line, "help", 4) == 0) {
        uart_puts("Commands: echo <text>, help");
    } else if (line[0] != '\0') {
        uart_puts("Unknown command.");
    }

    uart_puts("\r\n$ ");
}

void shell_init() {
    index = 0;
    uart_puts("[+] Entering main loop\r\n\nWelcome to the Shell!\r\nUse help to get started\r\n\n$ ");
}

void shell_update() {
    int c = uart_getc();
    if (c == -1) return; // No input, just return to kernel

    if (c == '\r') {
        buffer[index] = '\0';
        execute_command(buffer);
        index = 0;
    } else if (c == 0x7F || c == '\b') {
        if (index > 0) {
            index--;
            uart_puts("\b \b");
        }
    } else if (index < MAX_BUF - 1) {
        buffer[index++] = (char)c;
        uart_putc((char)c); // Visual echo
    }
}