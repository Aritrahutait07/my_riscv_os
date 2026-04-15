#ifndef UART_H
#define UART_H

#include <stdint.h>

// This is the "Blueprint" for our UART object
typedef struct {
    uint64_t base_address;
} Uart;

#define UART0_ADDR 0x10000000
#define UART_BASE 0x10000000
#define UART0 ((volatile uint8_t *)(UART0_ADDR))
#define UART_REG(offset) ((volatile uint8_t *)(UART_BASE + offset))

// List the "Menu" options
Uart uart_new(uint64_t base_address);
int uart_getc();
void uart_put(Uart *self, uint8_t c);
void uart_putc(char c);
void uart_puts(const char *s);
void uart_init();
void uart_write_str(Uart *self, const char *s);
// int  uart_get(Uart *self);
void mmio_write(uint64_t address, uint64_t offset, uint8_t value);
uint8_t mmio_read(uint64_t address, uint64_t offset);
void uart_puthex(uint64_t val);
void uart_printf(const char *fmt, ...);
void __attribute__((noreturn)) panic(const char *message, const char *file, uint32_t line);
#endif