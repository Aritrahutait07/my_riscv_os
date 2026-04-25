#include "trap.h"
#include <uart/uart.h>


uint64_t m_trap(uint64_t epc, uint64_t tval, uint64_t cause, uint64_t hart, uint64_t status, TrapFrame *frame){
    (void) status; // currently unused, but can be used in the future for more complex trap handling based on the CPU status at the time of the trap
    (void) frame; // currently unused, but can be used in the future to access and modify the CPU registers at the time of the trap for more advanced trap handling (e.g., emulating instructions, modifying return values, etc.)
    int is_async = (cause >> 63) & 1; // check if the trap is asynchronous (i.e., an interrupt)
    uint64_t cause_code = cause & 0x7FFFFFFFFFFFFFFF; // extract the cause code from the cause register (the lower 63 bits)
    uint64_t return_epc = epc; // initialize new_epc to the current epc, it can be modified based on the cause of the trap
    if(is_async){
        switch(cause_code){
            case 3:
                uart_printf("[+] Software Interrupt from hart %d\r\n", hart);
                break;
            case 7: {
                volatile uint64_t *mtime = (volatile uint64_t *)0x0200bff8; // Current time register of the CLINT (Core Local Interruptor) in the RISC-V architecture, which is used to keep track of the current time in the system.
 
                volatile uint64_t *mtimecmp = (volatile uint64_t *)0x02004000; // Compare register for hart 0 in the CLINT, which is used to set a timer interrupt for that hart. When the value in mtime reaches the value in mtimecmp, a timer interrupt is triggered for hart 0.
                *mtimecmp = *mtime + 1000000; // Set the timer interrupt to occur after a certain number of clock cycles (in this case, 1 million cycles) by writing
                break;
            }
            case 8:
                uart_printf("[+] External Interrupt from hart %d\r\n", hart);
                break;
            default:
               panic("Unhandled sync trap", __FILE__, __LINE__);
               break;
        }
    }else{
        switch(cause_code){
            case 2:
                uart_printf("Illegal instruction CPU#%d -> 0x%x: 0x%x\r\n", hart, epc, tval);
                panic("Illegal Instruction Trap", __FILE__, __LINE__);
                break;
            case 8:
                uart_printf("E-call from User mode! CPU#%d -> 0x%x\r\n", hart, epc);
                return_epc += 4;
                break;
            case 9:
                uart_printf("E-call from Supervisor mode! CPU#%d -> 0x%x\r\n", hart, epc);
                return_epc += 4;
                break;
            case 11:
                uart_printf("E-call from Machine mode! CPU#%d -> 0x%x\r\n", hart, epc);
                panic("Machine E-Call", __FILE__, __LINE__);
                break;
            case 12:
                uart_printf("Instruction page fault CPU#%d -> 0x%x: 0x%x\r\n", hart, epc, tval);
                return_epc += 4;
                break;
            case 13:
                uart_printf("Load page fault CPU#%d -> 0x%x: 0x%x\r\n", hart, epc, tval);
                return_epc += 4;
                break;
            case 15:
                uart_printf("Store page fault CPU#%d -> 0x%x: 0x%x\r\n", hart, epc, tval);
                return_epc += 4;
                break;
            default:
                uart_printf("\r\n!!! UNHANDLED SYNC TRAP !!!\r\n");
                uart_printf("Exception Code: %d\r\n", cause_code);
                uart_printf("Faulting PC   : 0x%x\r\n", epc);
                uart_printf("Faulting Value: 0x%x\r\n", tval); // mtval
                panic("System Halted", __FILE__, __LINE__);
                break;
        }
    }
    return return_epc; // return the new program counter to resume execution after handling the trap

}
