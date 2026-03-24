#include <stdint.h>
// #include "uart.h" 

// Using a macro is the "Best Practice" for hardware addresses
#define UART0_ADDR 0x10000000
#define UART_BASE 0x10000000
#define UART0 ((volatile uint8_t *)(UART0_ADDR))
#define UART_REG(offset) ((volatile uint8_t *)(UART_BASE + offset))

#define print(fmt, ...)   do { } while (0)
#define println(fmt, ...) do { } while (0)

typedef struct {
    uint64_t base_address;
} Uart;

Uart uart_new(uint64_t base_address) {
    Uart u;
    u.base_address = base_address;
    return u;
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

void uart_init(Uart *self) {
    volatile uint8_t *ptr = (volatile uint8_t *)self->base_address;
    ptr[3] = 0x03; // Set 8-bit word length
    ptr[2] = 0x01; // Enable FIFO
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

// void uart_init(){
//     // *UART_REG(3) = 0x03;
//     // *UART_REG(2) = 0x01;
//     // *UART_REG(1) = 0x00;
//     uint8_t lcr = (1 << 0) | (1 << 1);
//     *UART_REG(3) = lcr;
//     *UART_REG(2) = (1 << 0);
//     *UART_REG(1) = (1 << 0);
//     *UART_REG(3) = lcr | (1 << 7);
//     uint16_t divisor = 592;
//     *UART_REG(0) = (uint8_t)(divisor & 0xFF);         
//     *UART_REG(1) = (uint8_t)((divisor >> 8) & 0xFF); 
//     *UART_REG(3) = lcr;
// }
// int uart_getc() {
//     if ((*UART_REG(5) & (1 << 0)) == 0) {
//         return -1; // No data to read
//     } else {
//         return *UART_REG(0);
//     }
// }

// void uart_putc(char c) {
//     while ((*UART_REG(5) & 0x20) == 0) {
//     }
//     *UART_REG(0) = (uint8_t)c;
// }


// void uart_puts(const char *s) {
//     while (*s) {
//         uart_putc(*s++);
//     }
// }

void mmio_write(uint64_t address, uint64_t offset, uint8_t value) {
    volatile uint8_t *reg = (volatile uint8_t *)(address + offset);
    *reg = value;
}
uint8_t mmio_read(uint64_t address, uint64_t offset) {
    volatile uint8_t *reg = (volatile uint8_t *)(address + offset);
    return *reg;
}

void kmain(void) {
    // uart_init();
    // uart_puts("Hello, RISC-V World!\n");
    // uart_puts("MMIO is working correctly at 0x10000000.\n");

    mmio_write(0x10000000, 0, 'A');
    uint8_t status = mmio_read(0x10000000, 5);

    if (status & 0x20) {
        uart_puts("UART ready!\n");
    } else {
        uart_puts("UART not ready!\n");
    }

    Uart my_console = uart_new(0x10000000);
    uart_init(&my_console);
    uart_write_str(&my_console, "This is organized C code!\n");
    uart_write_str(&my_console, "We are using Abstraction to talk to hardware.\n");
    
    
    

    while (1) {
         __asm__ volatile("wfi");

    //    int c = uart_getc();
    //     if (c != -1) {
            
    //         uart_putc(c);
            
           
    //         if (c == '\r') uart_putc('\n');
    //     }
    }
}

