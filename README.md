# 🛰️ Bare-Metal RISC-V Operating System
**A custom OS kernel built from scratch in C for the RISC-V architecture.**

## 📌 Project Overview
This project is a journey into the "heart of the machine." I am developing a bare-metal kernel targeting the **RISC-V (RV64)** architecture using **QEMU (virt machine)**. The goal is to understand kernel-level memory management, process scheduling, and hardware-software interfacing without the abstraction of a standard library.

## 🛠️ Tech Stack
- **Language:** C, RISC-V Assembly
- **Environment:** QEMU (virt machine)
- **Toolchain:** `riscv64-unknown-elf-gcc`, `gdb`, `make`

## 🚀 Key Features (In Development)
- [x] **Bootloader Logic:** Transitioning from Assembly to C execution.
- [x] **UART Driver:** Basic I/O for serial console communication.
- [ ] **Memory Management:** Implementing a Page Allocator and Virtual Memory (Sv39).
- [ ] **Interrupt Handling:** Managing hardware traps and PLIC (Platform-Level Interrupt Controller).

## 📂 Project Structure
- `boot.S`: Entry point and stack initialization.
- `kernel.c`: Main kernel logic and UART initialization.
- `linker.ld`: Defines the memory layout for the RISC-V virt machine.

## 💻 How to Run
1. Ensure you have the RISC-V toolchain and QEMU installed.
2. Clone the repo: `git clone https://github.com/Aritrahutait07/my_riscv_os`
3. Run using QEMU:
   ```bash
   qemu-system-riscv64 -machine virt -cpu rv64 -nographic -serial mon:stdio -kernel kernel.elf
