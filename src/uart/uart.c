#include <stdint.h>
#include <stdarg.h>
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

void __attribute__((noreturn)) panic(const char *message, const char *file, uint32_t line){
    uart_puts("\r\n--- KERNEL PANIC ---\r\n");
    uart_printf("Message: %s\r\n", message);
    uart_printf("At: %s:%d\r\n", file, line);
    uart_puts("----------------------\r\n");
    uart_puts("System halted.\r\n");
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

void uart_puthex(uint64_t val) {
    char *hex = "0123456789abcdef";
    for (int i = 60; i >= 0; i -= 4) {
        // (val >> i) extracts the current 4 bits, & 0xf ensures we only get the last 4 bits, which is the hex digit
        uart_putc(hex[(val >> i) & 0xf]);
    }
}


void uart_printf(const char *fmt, ...){
    va_list args;
    va_start(args, fmt);
    for(const char *p = fmt; *p != '\0'; p++){
        if(*p!='%'){
            uart_putc(*p);
            continue;
        }
        p++;
            switch(*p){
                case 's': {
                    const char *str = va_arg(args, const char *);
                    uart_puts(str);
                    break;
                }
                case 'd': {
                    //int num = va_arg(args, int);
                    // change to long long to support 64-bit integers in RISC-V as int is 32-bit in C
                    long long num = va_arg(args, long long);
                    if(num < 0){
                        uart_putc('-');
                        num = -num;
                    }
                    char buf[21]; // enough to hold 64-bit integer in decimal 
                    int i = 0;
                    do {
                        buf[i++] = (num % 10) + '0';
                        num /= 10;
                    } while(num > 0);
                    for(int j = i-1; j >= 0; j--){
                        uart_putc(buf[j]);
                    }
                    break;
                }
                case 'x':{
                    uint64_t num = va_arg(args, uint64_t);
                    // uart_puts("0x");
                    uart_puthex(num);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    uart_putc(c);
                    break;
                }
                case 'p':{
                    uint64_t num = va_arg(args, uint64_t);
                    uart_puts("0x");
                    uart_puthex(num);
                    break;
                }
                default:
                    uart_putc('%');
                    uart_putc(*p);

            }

    }
    va_end(args);

}
