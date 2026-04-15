#ifndef TRAP_H
#define TRAP_H
#include <stdint.h>

typedef struct {
    uint64_t regs[32]; // Each register is 64 bits and  this eventually will be used to store the state of the CPU when a trap occurs
    uint64_t fregs[32]; // Floating-point registers, which can be used to store the state of the floating-point unit (FPU) at the time of the trap, allowing for more comprehensive trap handling that includes floating-point operations.
    uint64_t satp; // The SATP (Supervisor Address Translation and Protection) register, which is used in RISC-V to manage virtual memory and address translation. Including this in the TrapFrame allows the trap handler to have access to the current memory management state of the CPU, which can be crucial for handling traps related to memory access (e.g., page faults) and for implementing features like virtual memory.
    void*    trap_stack; // A pointer to the trap stack, which can be used to store the state of the CPU and other relevant information when a trap occurs. This allows the trap handler to have a dedicated area of memory for handling traps, which can help prevent corruption of the main stack and improve the reliability of trap handling.
    
} TrapFrame;

// The m_trap function is the main trap handler for the operating system. It is called whenever a trap occurs, and it takes several parameters that provide information about the trap and the state of the CPU at the time of the trap. The function can be used to handle different types of traps (e.g., interrupts, exceptions, etc.) and to perform necessary actions based on the cause of the trap and the state of the CPU.
/*
epc -> the program counter at the time of the trap, which can be used to determine where the trap occurred in the code.
tval -> a value that provides additional information about the trap, such as the address that caused
cause -> the reason for the trap, which can be used to determine the type of trap that occurred (e.g., an interrupt, an exception, etc.)
hart -> the hardware thread (or core) that was executing when the trap occurred, which can  be useful for handling traps in a multi-core system.
status -> the status register at the time of the trap, which can provide information about the state of the CPU and the trap.
frame -> a pointer to a TrapFrame structure that can be used
*/
uint64_t m_trap(uint64_t epc, uint64_t tval, uint64_t cause, uint64_t hart, uint64_t status, TrapFrame *frame);

#endif