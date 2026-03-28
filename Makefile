CC = riscv64-unknown-elf-gcc
# Added -mcmodel=medany to fix the relocation error
# Added -Wl,--no-warn-rwx-segments to hide the linker warning
CFLAGS = -Wall -Wextra -march=rv64gc -mabi=lp64d -mcmodel=medany -ffreestanding -nostdlib -nostartfiles -Isrc -Wl,--no-warn-rwx-segments

all: os.elf

os.elf: src/asm/boot.S src/asm/trap.S src/kernel.c src/uart/uart.c src/shell/shell.c
	$(CC) $(CFLAGS) -Tsrc/lds/virt.lds src/asm/boot.S src/asm/trap.S src/kernel.c src/uart/uart.c src/shell/shell.c src/pages/page.c -o os.elf

run: all
	qemu-system-riscv64 -machine virt -cpu rv64 -smp 4 -m 128M -nographic -serial mon:stdio -bios none -kernel os.elf

clean:
	rm -f os.elf