.section .text
.global asm_trap_vector
.align 4

asm_trap_vector:
    # 1. Load the Trap data into the Argument Registers (a0 - a5)
    # The C function signature is:
    # m_trap(a0=epc, a1=tval, a2=cause, a3=hart, a4=status, a5=frame)
    
    csrr a0, mepc      # a0 = epc
    csrr a1, mtval     # a1 = tval
    csrr a2, mcause    # a2 = cause
    csrr a3, mhartid   # a3 = hart
    csrr a4, mstatus   # a4 = status
    mv   a5, sp        # a5 = frame (We'll use Stack Pointer for now)

    # 2. Call the C function
    call m_trap

    # 3. m_trap returns the new PC in register 'a0'.
    # We must write it back to MEPC so the CPU knows where to jump back to!
    csrw mepc, a0

    # 4. Return from trap
    mret