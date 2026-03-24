#ifndef UART_H
#define UART_H

#include <stdint.h>

// This is the "Blueprint" for our UART object
typedef struct {
    uint64_t base_address;
} Uart;

// List the "Menu" options
Uart uart_new(uint64_t base_address);
// void uart_init(Uart *self);
void uart_put(Uart *self, uint8_t c);
void uart_write_str(Uart *self, const char *s);
// int  uart_get(Uart *self);

#endif