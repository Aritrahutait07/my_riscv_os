#include <stdint.h>
#include "uart.h" 

#define print(fmt, ...)   do { } while (0)
#define println(fmt, ...) do { } while (0)

Uart uart_new(uint64_t base_address) {
    Uart u;
    u.base_address = base_address;
    return u;
}

int uart_getc() {
    if (*UART_REG(5) & 0x01) { // Data Ready?
        return *UART_REG(0);
    }
    return -1;
}

void uart_put(Uart *self, uint8_t c) {
    volatile uint8_t *ptr = (volatile uint8_t *)self->base_address;

    // Wait until the transmitter is empty (Bit 5 of Status Register)
    while ((*(ptr + 5) & 0x20) == 0) {
        // Wait...
    }

    // Write the byte to Offset 0
    *(ptr + 0) = c;
}

void uart_putc(char c) {
    // Wait until Transmitter Holding Register is Empty (Bit 5 of LSR)
    while ((*UART_REG(5) & 0x20) == 0);
    *UART_REG(0) = (uint8_t)c;
}

void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}

void uart_init() {
    // 1. Disable all interrupts in UART hardware
    *UART_REG(1) = 0x00;
    
    // 2. Set Baud Rate (not strictly needed for QEMU, but good practice)
    *UART_REG(3) = 0x80; // Enable DLAB
    *UART_REG(0) = 0x03; // DLL (Divisor Latch Low)
    *UART_REG(1) = 0x00; // DLM (Divisor Latch High)
    
    // 3. Set word length to 8-bits, no DLAB
    *UART_REG(3) = 0x03;
    
    // 4. Enable FIFO, clear them
    *UART_REG(2) = 0x07;
}


void uart_write_str(Uart *self, const char *s) {
    while (*s) {
        uart_put(self, (uint8_t)*s++);
    }
}

void __attribute__((noreturn)) abort() {
    while (1) {
        __asm__ volatile("wfi");
    }
}

void panic(const char *message, const char *file, uint32_t line) {
    (void)message; 
    (void)file;
    (void)line;

    abort();
}

void mmio_write(uint64_t address, uint64_t offset, uint8_t value) {
    volatile uint8_t *reg = (volatile uint8_t *)(address + offset);
    *reg = value;
}
uint8_t mmio_read(uint64_t address, uint64_t offset) {
    volatile uint8_t *reg = (volatile uint8_t *)(address + offset);
    return *reg;
}

