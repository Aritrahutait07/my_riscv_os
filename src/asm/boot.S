# Tells assembler to use 32-bit code don't use compressed instructions
.option norvc

.section .text.init

.globl _start
_start:
    csrr t0 , mhartid
    bnez t0, 3f
    csrw satp, zero
.option push
.option norelax
	la		gp, _global_pointer
.option pop
    la   a0, _bss_start
    la   a1, _bss_end

    bgeu  a0, a1, 2f
1:
    sd  zero, 0(a0)
    addi a0, a0, 8
    blt   a0, a1, 1b
2:
    la  sp, _stack_end
    # li      t0, (0b11 << 11) | (1 << 7) | (1 << 3)
    li      t0, (0b11 << 11) 
    csrw    mstatus, t0

    la      t1, kinit       
    csrw    mepc, t1
    la      t2, asm_trap_vector
    csrw    mtvec, t2
    # li      t3, (1 << 3) | (1 << 7) | (1 << 11)
    # csrw    mie, t3
    csrw    mie, zero
    la      ra, 4f
    mret


3:
	wfi
	j	3b

4:
    wfi              
    j       4b
