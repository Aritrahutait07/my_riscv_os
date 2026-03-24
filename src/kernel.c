#include <stdint.h>
#include <stdarg.h>
// #include "uart.h" 

// Using a macro is the "Best Practice" for hardware addresses
#define UART0_ADDR 0x10000000
#define UART_BASE 0x10000000
#define UART0 ((volatile uint8_t *)(UART0_ADDR))
#define UART_REG(offset) ((volatile uint8_t *)(UART_BASE + offset))

// Corrected Macros:
// We moved the working println below the empty one so the compiler uses the working one.
#define print(fmt, ...)   do { } while (0)
#define println_empty(fmt, ...) do { } while (0) 
#define println(fmt, ...) kprintf(fmt "\r\n", ##__VA_ARGS__)

typedef struct {
    uint64_t base_address;
} Uart;

static Uart GLOBAL_CONSOLE;

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
    // ptr[3] = 0x03; // Set 8-bit word length
    // ptr[2] = 0x01; // Enable FIFO
    ptr[3] = 0x03; ptr[2] = 0x01; ptr[1] = 0x01;
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
int uart_getc() {
    if ((*UART_REG(5) & (1 << 0)) == 0) {
        return -1; // No data to read
    } else {
        return *UART_REG(0);
    }
}

void uart_putc(char c) {
    while ((*UART_REG(5) & 0x20) == 0) {
    }
    *UART_REG(0) = (uint8_t)c;
}

// Moved kprintf definition here so println can see it
void kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (const char *p = fmt; *p != '\0'; p++) {
        if (*p != '%') {
            uart_put(&GLOBAL_CONSOLE, *p);
            continue;
        }

        p++; // Move past '%'
        switch (*p) {
            case 's': { // String
                char *s = va_arg(args, char *);
                uart_write_str(&GLOBAL_CONSOLE, s);
                break;
            }
            case 'd': { // Integer
                int n = va_arg(args, int);
                if (n == 0) {
                    uart_put(&GLOBAL_CONSOLE, '0');
                } else {
                    char buf[12]; // Max digits for 32-bit int
                    int i = 0;
                    while (n > 0) {
                        buf[i++] = (n % 10) + '0';
                        n /= 10;
                    }
                    while (i > 0) uart_put(&GLOBAL_CONSOLE, buf[--i]);
                }
                break;
            }
            case 'c': { // Character
                char c = (char)va_arg(args, int);
                uart_put(&GLOBAL_CONSOLE, c);
                break;
            }
            case '%': {
                uart_put(&GLOBAL_CONSOLE, '%');
                break;
            }
        }
    }
    va_end(args);
}

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
    // 1. Create the UART object first so we can use it
    GLOBAL_CONSOLE.base_address = 0x10000000;
    Uart my_console = uart_new(0x10000000); // Uncommented this so &my_console works below
    uart_init(&GLOBAL_CONSOLE);

    // 2. Test manual MMIO write
    mmio_write(0x10000000, 0, 'A');
    
    // 3. Test manual MMIO read
    uint8_t status = mmio_read(0x10000000, 5);

    // FIX: Changed 'uart_puts' to 'uart_write_str' and passed '&my_console'
    if (status & 0x20) {
        uart_write_str(&my_console, "\nUART ready!\n");
    } else {
        uart_write_str(&my_console, "\nUART not ready!\n");
    }

    // 4. Test the organized structure logic
    uart_write_str(&my_console, "This is organized C code!\n");
    uart_write_str(&my_console, "We are using Abstraction to talk to hardware.\n");

    int cpu_id = 0;

    // kprintf(&my_console, "Booting OS...\n");
    // kprintf(&my_console, "Running on Hart ID: %d\n", cpu_id);
    // kprintf(&my_console, "Status: %s\n", "All systems nominal");

    println(""); // Arm 1: Just a newline
    println("This is my operating system! "); // Arm 2: Simple string
    println("System Status: %s", "Running"); // Arm 3: Formatting

    println("Type into the terminal to test the echo:");
    println("Running on Hart (CPU) ID: %d", cpu_id); // Arm 4: More complex formatting

    while (1) {
        // We check for data continuously (Polling)
        volatile uint8_t *ptr = (volatile uint8_t *)0x10000000;
        if (ptr[5] & 0x01) { // If Data Ready
            uint8_t c = ptr[0];
            uart_put(&GLOBAL_CONSOLE, c); // Echo
            if (c == '\r') uart_put(&GLOBAL_CONSOLE, '\n');
        }

        // We can use wfi here to save power, but polling is better for testing echo right now
        // __asm__ volatile("wfi");

    //    int c = uart_getc();
    //     if (c != -1) {
    //         uart_putc(c);
    //         if (c == '\r') uart_putc('\n');
    //     }
    }
}